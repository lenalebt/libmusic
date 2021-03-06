#include "timbre.hpp"

#include "debug.hpp"
#include <Eigen/Dense>
#include <limits>

namespace music
{
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> TimbreEstimator::estimateTimbre(double fromTime, double toTime)
    {
        assert (fromTime <= toTime);
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec(transformResult->getOctaveCount() * transformResult->getBinsPerOctave());
        
        int i=0;
        double duration = toTime - fromTime;
        double sum=0.0, sumEl, max=0.0;
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                //when taking the mean values, absolute values are returned.
                vec[i] = log(sumEl=transformResult->getNoteValueMean(toTime, octave, bin, duration));
                sum += sumEl;
                if (sumEl > max)
                    max = sumEl;
                
                if (vec[i] < -100)
                    vec[i] = -100;
                
                i++;
            }
        }
        
        //on low values, do not calculate timbre. instead, give back an error value.
        if (sum < minEnergy)
            return Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>::Zero(1);
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> timbre(timbreVectorSize);
        //apply dct...
        for (unsigned int k=1; k<=timbreVectorSize; k++)
        {
            double sum=0.0;
            for (int n=0; n<cosValues.rows(); n++)
            {
                sum += vec[n] * cosValues(n, k-1);
            }
            timbre[k-1] = sum;
        }
        return timbre;
    }
    
    TimbreEstimator::TimbreEstimator(ConstantQTransformResult* transformResult, unsigned int timbreVectorSize, float minEnergy) :
        transformResult(transformResult), timbreVectorSize(timbreVectorSize), cosValues(transformResult->getOctaveCount() * transformResult->getBinsPerOctave(), timbreVectorSize), minEnergy(minEnergy)
    {
        assert(transformResult != NULL);
        assert(timbreVectorSize > 1);
        assert(timbreVectorSize <= (unsigned int)transformResult->getOctaveCount() * transformResult->getBinsPerOctave());
        for (unsigned int k=1; k<=timbreVectorSize; k++)
        {
            for (int n=0; n<cosValues.rows(); n++)
            {
                cosValues(n, k-1) = sqrt(2.0/double(cosValues.rows())) * cos(M_PI/double(cosValues.rows()) * (double(n)+0.5) * double(k));
            }
        }
    }
    
    TimbreModel::TimbreModel(ConstantQTransformResult* transformResult) :
        transformResult(transformResult), model(NULL)
    {
        
    }
    TimbreModel::~TimbreModel()
    {
        if (model)
            delete model;
    }
    bool TimbreModel::calculateTimbreVectors(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& timbreVectors, double timeSliceSize, unsigned int timbreVectorSize)
    {
        assert(timeSliceSize > 0.0);
        assert(timbreVectorSize > 1);
        
        unsigned int foundTimbreVectors=0;
        TimbreEstimator tEst(transformResult, timbreVectorSize);
        double time;
        for (int i=1; i<transformResult->getOriginalDuration()/timeSliceSize; i++)
        {
            time = i*timeSliceSize;
            Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmp = tEst.estimateTimbre(time - timeSliceSize, time);
            if (tmp.size() > 1)
            {
                timbreVectors.push_back(tmp);
                foundTimbreVectors++;
            }
        }
        return foundTimbreVectors>0;
    }
    bool TimbreModel::calculateModel(unsigned int modelSize, double timeSliceSize, unsigned int timbreVectorSize, ProgressCallbackCaller* callback)
    {
        //use overload to hide the possibility of getting the data vectors.
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > data;
        return calculateModel(data, modelSize, timeSliceSize, timbreVectorSize, callback);
    }
    bool TimbreModel::calculateModel(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& timbreVectors, unsigned int modelSize, double timeSliceSize, unsigned int timbreVectorSize, ProgressCallbackCaller* callback)
    {
        assert(modelSize > 0);
        assert(timeSliceSize > 0.0);
        assert(timbreVectorSize > 1);
        
        if (model)
        {
            delete model;
            model = NULL;
        }
        model = new GaussianMixtureModelDiagCov<kiss_fft_scalar>();
        
        if (callback)
            callback->progress(0.0, "initialized");
        
        //first get data vectors
        if (timbreVectors.empty())
        {
            assert(transformResult != NULL);
            TimbreEstimator tEst(transformResult, timbreVectorSize);
            double time;
            for (int i=1; i<transformResult->getOriginalDuration()/timeSliceSize; i++)
            {
                time = i*timeSliceSize;
                Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmp = tEst.estimateTimbre(time - timeSliceSize, time);
                if (tmp.size() > 1)
                    timbreVectors.push_back(tmp);
            }
            if (callback)
                callback->progress(0.5, "calculated timbre vectors, training model now");
        }
        else
        {
            if (callback)
                callback->progress(0.5, "used old timbre vectors, training model now");
        }
        
        if (timbreVectors.size() < timbreVectorSize)
            return false;
        
        //then train the model (best-of-three).
        if (callback)
            callback->progress(0.5,  "calculating model 1");
        model->trainGMM(timbreVectors, modelSize);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel1 = model->clone();
        if (callback)
            callback->progress(0.65, "calculating model 2");
        model->trainGMM(timbreVectors, modelSize);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel2 = model->clone();
        if (callback)
            callback->progress(0.8,  "calculating model 3");
        model->trainGMM(timbreVectors, modelSize);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel3 = model->clone();
        
        delete model;
        model = NULL;
        
        if (callback)
            callback->progress(0.95, "choose best model");
        
        //choose best-of-three
        if (tmpModel1->getModelLogLikelihood() > tmpModel2->getModelLogLikelihood())
        {
            if (tmpModel1->getModelLogLikelihood() > tmpModel3->getModelLogLikelihood())
            {
                model = tmpModel1;
                delete tmpModel2;
                delete tmpModel3;
            }
            else
            {
                model = tmpModel3;
                delete tmpModel1;
                delete tmpModel2;
            }
        }
        else
        {
            if (tmpModel2->getModelLogLikelihood() > tmpModel3->getModelLogLikelihood())
            {
                model = tmpModel2;
                delete tmpModel1;
                delete tmpModel3;
            }
            else
            {
                model = tmpModel3;
                delete tmpModel1;
                delete tmpModel2;
            }
        }
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return true;
    }
    GaussianMixtureModel<kiss_fft_scalar>* TimbreModel::getModel()
    {
        return model;
    }
}

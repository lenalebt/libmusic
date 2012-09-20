#include "timbre.hpp"

#include "debug.hpp"
#include <Eigen/Dense>
#include <limits>

namespace music
{
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> TimbreEstimator::estimateTimbre(double fromTime, double toTime)
    {
        return this->estimateTimbre4(fromTime, toTime);
    }
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> TimbreEstimator::estimateTimbre4(double fromTime, double toTime)
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
        DEBUG_VAR_OUT(timbre.transpose(), 0);
        return timbre;
    }
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> TimbreEstimator::estimateTimbre3(double fromTime, double toTime)
    {
        assert (fromTime <= toTime);
        
        kiss_fft_scalar vec[128];
        
        //we need less than 128 bins, otherwise we might not use the stack to store the arrays.
        assert(transformResult->getOctaveCount() * transformResult->getBinsPerOctave() <= 128);
        
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
        
        
        DEBUG_VAR_OUT(sum, 0);
        DEBUG_VAR_OUT(max, 0);
        if (sum < 5.0)
            return Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>::Zero(1);
        
        
        kiss_fft_scalar freqData[128];
        //apply dct...
        dct.doDCT2(vec, transformResult->getOctaveCount() * transformResult->getBinsPerOctave(), freqData);
        //TODO: now we are calculating way too much. use the scalar product version and only calculate some part of the signal...
        
        /*
        for (i=0; i<transformResult->getOctaveCount() * transformResult->getBinsPerOctave(); i++)
            std::cerr << vec[i] << " ";
        std::cerr << std::endl;
        for (i=0; i<transformResult->getOctaveCount() * transformResult->getBinsPerOctave(); i++)
            std::cerr << freqData[i] << " ";
        std::cerr << std::endl;*/
        
        int timbreVectorSize = 12;
        //int timbreVectorSize = 128;
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> timbre(timbreVectorSize);
        for (int i=1; i<timbreVectorSize+1; i++)
        //for (int i=0; i<timbreVectorSize; i++)
        //    timbre[i] = freqData[i];
            timbre[i-1] = freqData[i];
        DEBUG_VAR_OUT(timbre.transpose(), 0);
        return timbre;
    }
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> TimbreEstimator::estimateTimbre2(double fromTime, double toTime)
    {
        assert (fromTime <= toTime);
        
        kiss_fft_scalar vec[128];
        
        int i=0;
        double duration = toTime - fromTime;
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                //when taking the mean values, absolute values are returned.
                vec[i] = log(transformResult->getNoteValueMean(toTime, octave, bin, duration));
                
                if (vec[i] < -100)
                    vec[i] = -100;
                if ((vec[i] > 10000) || (vec[i] != vec[i]))
                {
                    DEBUG_VAR_OUT(vec[i], 0);
                    DEBUG_VAR_OUT(transformResult->getNoteValueMean(toTime, octave, bin, duration), 0);
                    ERROR_OUT("stop! something bad!", 0);
                    return Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>();
                }
                
                //std::cerr << vec[i] << " ";
                i++;
            }
        }
        //std::cerr << std::endl;
        //set the rest to log(zero)...
        for (; i<128; i++)
            vec[i] = -100.0f;
        
        kiss_fft_scalar freqData[128];
        //apply dct...
        dct.doDCT2(vec, 128, freqData);
        
        for (i=0; i<128; i++)
            std::cerr << vec[i] << " ";
        std::cerr << std::endl;
        for (i=0; i<128; i++)
            std::cerr << freqData[i] << " ";
        std::cerr << std::endl;
        
        int timbreVectorSize = 12;
        //int timbreVectorSize = 128;
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> timbre(timbreVectorSize);
        for (int i=1; i<timbreVectorSize+1; i++)
        //for (int i=0; i<timbreVectorSize; i++)
        //    timbre[i] = freqData[i];
            timbre[i-1] = freqData[i];
        DEBUG_VAR_OUT(timbre.transpose(), 0);
        return timbre;
    }
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> TimbreEstimator::estimateTimbre1(double fromTime, double toTime)
    {
        assert (fromTime <= toTime);
        
        kiss_fft_scalar vec[128];
        
        int i=0;
        double duration = toTime - fromTime;
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                //when taking the mean values, absolute values are returned.
                vec[i] = transformResult->getNoteValueMean(toTime, octave, bin, duration);
                vec[i] = log(vec[i]);
                if (vec[i] < -100)
                    vec[i] = -100;
                std::cerr << vec[i] << " ";
                i++;
            }
        }
        std::cerr << std::endl;
        //set the rest to zero...
        for (; i<128; i++)
            vec[i] = -100.0f;
        
        kiss_fft_scalar freqData[128];
        //apply dct...
        dct.doDCT2(vec, 128, freqData);
        
        //for (i=0; i<128; i++)
        //    std::cerr << vec[i] << " ";
        //for (i=0; i<128; i++)
        //    std::cerr << freqData[i] << " ";
        //std::cerr << std::endl;
        
        int timbreVectorSize = 12;
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> timbre(timbreVectorSize);
        DEBUG_VAR_OUT(freqData[0], 0);
        for (int i=1; i<timbreVectorSize+1; i++)
            timbre[i-1] = freqData[i];
        DEBUG_VAR_OUT(timbre.transpose(), 0);
        return timbre;
    }
    
    TimbreEstimator::TimbreEstimator(ConstantQTransformResult* transformResult, unsigned int timbreVectorSize, float minEnergy) :
        transformResult(transformResult), timbreVectorSize(timbreVectorSize), cosValues(transformResult->getOctaveCount() * transformResult->getBinsPerOctave(), timbreVectorSize), minEnergy(minEnergy)
    {
        assert(timbreVectorSize > 1);
        for (unsigned int k=1; k<=timbreVectorSize; k++)
        {
            for (int n=0; n<cosValues.rows(); n++)
            {
                cosValues(n, k-1) = cos(M_PI/double(cosValues.rows()) * (double(n)+0.5) * double(k));
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
    void TimbreModel::calculateModel(int modelSize, double timeSliceSize, unsigned int timbreVectorSize, ProgressCallbackCaller* callback)
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
        TimbreEstimator tEst(transformResult, timbreVectorSize);
        double time;
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > data;
        for (int i=1; i<transformResult->getOriginalDuration()/timeSliceSize; i++)
        {
            time = i*timeSliceSize;
            data.push_back(tEst.estimateTimbre(time - timeSliceSize, time));
        }
        if (callback)
            callback->progress(0.5, "calculated timbre vectors");
        
        //then train the model
        model->trainGMM(data, modelSize);
        
        if (callback)
            callback->progress(1.0, "finished");
    }
    GaussianMixtureModel<kiss_fft_scalar>* TimbreModel::getModel()
    {
        return model;
    }
}

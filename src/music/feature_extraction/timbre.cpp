#include "timbre.hpp"

#include "debug.hpp"
#include <Eigen/Dense>
#include <limits>

namespace music
{
    Eigen::VectorXd TimbreEstimator::estimateTimbre(double fromTime, double toTime)
    {
        return this->estimateTimbre2(fromTime, toTime);
    }
    Eigen::VectorXd TimbreEstimator::estimateTimbre2(double fromTime, double toTime)
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
                i++;
            }
        }
        //set the rest to zero...
        for (; i<128; i++)
            vec[i] = -100.0f;
        
        kiss_fft_scalar freqData[128];
        //apply dct...
        //dct.doDCT2(vec, 128, freqData);
        for (int i=0; i<128; i++)
        {
            double sum=0.0;
            for (int j=0; j<128; j++)
            {
                sum += vec[j] * cos(double(i)*(double(j)-0.5)*M_PI/128.0);
            }
            freqData[i] = sum;
        }
        
        //for (i=0; i<128; i++)
        //    std::cerr << vec[i] << " ";
        //for (i=0; i<128; i++)
        //    std::cerr << freqData[i] << " ";
        //std::cerr << std::endl;
        
        int timbreVectorSize = 12;
        //int timbreVectorSize = 128;
        Eigen::VectorXd timbre(timbreVectorSize);
        DEBUG_VAR_OUT(freqData[0], 0);
        for (int i=1; i<timbreVectorSize+1; i++)
        //for (int i=0; i<timbreVectorSize; i++)
        //    timbre[i] = freqData[i];
            timbre[i-1] = freqData[i];
        DEBUG_VAR_OUT(timbre.transpose(), 0);
        return timbre;
    }
    Eigen::VectorXd TimbreEstimator::estimateTimbre1(double fromTime, double toTime)
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
        Eigen::VectorXd timbre(timbreVectorSize);
        DEBUG_VAR_OUT(freqData[0], 0);
        for (int i=1; i<timbreVectorSize+1; i++)
            timbre[i-1] = freqData[i];
        DEBUG_VAR_OUT(timbre.transpose(), 0);
        return timbre;
    }
    
    TimbreEstimator::TimbreEstimator(ConstantQTransformResult* transformResult) :
        transformResult(transformResult)
    {
        
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
    void TimbreModel::calculateModel(int modelSize, double timeSliceSize)
    {
        assert(modelSize > 0);
        assert(timeSliceSize > 0.0);
        
        if (model)
        {
            delete model;
            model = NULL;
        }
        model = new GaussianMixtureModelFullCov<double>();
        
        //first get data vectors
        TimbreEstimator tEst(transformResult);
        double time;
        std::vector<Eigen::VectorXd> data;
        for (int i=1; i<transformResult->getOriginalDuration()/timeSliceSize; i++)
        {
            time = i*timeSliceSize;
            data.push_back(tEst.estimateTimbre(time - timeSliceSize, time));
        }
        
        //then train the model
        model->trainGMM(data, modelSize);
    }
    GaussianMixtureModel<double>* TimbreModel::getModel()
    {
        return model;
    }
}

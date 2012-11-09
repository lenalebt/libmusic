#include "timbre.hpp"

#include "debug.hpp"
#include <Eigen/Dense>

namespace music
{
    Eigen::VectorXd TimbreEstimator::estimateTimbre(double fromTime, double toTime)
    {
        assert (fromTime <= toTime);
        
        float vec[128];
        
        int i=0;
        double duration = toTime - fromTime;
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                //when taking the mean values, absolute values are returned.
                vec[i] = transformResult->getNoteValueMean(toTime, octave, bin, duration);
                i++;
            }
        }
        //set the rest to zero...
        for (; i<128; i++)
            vec[i] = 0.0f;
        
        float freqData[128];
        //apply dct...
        dct.doDCT2(vec, 128, freqData);
        
        //for (i=0; i<128; i++)
        //    std::cerr << vec[i] << " ";
        //for (i=0; i<128; i++)
        //    std::cerr << freqData[i] << " ";
        //std::cerr << std::endl;
        
        int timbreVectorSize = 8;
        Eigen::VectorXd timbre(timbreVectorSize);
        for (int i=1; i<timbreVectorSize+1; i++)
            timbre[i-1] = freqData[i];
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

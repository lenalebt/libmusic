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
        
        Eigen::VectorXd timbre(20);
        for (int i=1; i<21; i++)
            timbre[i-1] = freqData[i];
        return timbre;
    }
    
    TimbreEstimator::TimbreEstimator(ConstantQTransformResult* transformResult) :
        transformResult(transformResult)
    {
        
    }
}

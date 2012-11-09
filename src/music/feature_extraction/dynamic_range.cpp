#include "dynamic_range.hpp"

#include <cmath>
#include <limits>

namespace music
{
    template <typename ScalarType>
    DynamicRangeCalculator<ScalarType>::DynamicRangeCalculator(PerTimeSliceStatistics<ScalarType>* perTimeSliceStatistics) :
        ptss(perTimeSliceStatistics),
        loudnessMean(0.0),
        loudnessRMS(0.0),
        loudnessVariance(0.0),
        loudnessRMSVariance(0.0)
    {
        
    }
    
    template <typename ScalarType>
    void DynamicRangeCalculator<ScalarType>::calculateDynamicRange(double doNotCountLastSeconds)
    {
        ptss->calculateSum();
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> sumVec = *(ptss->getSumVector());
        
        //do not count the last 20 seconds as this could be blended out
        int lastSumVecElement = sumVec.size();
        if (sumVec.size() * ptss->getTimeResolution() > 120.0)
            lastSumVecElement -= doNotCountLastSeconds/ptss->getTimeResolution();
        
        //first normalize our sum vector.
        double maxVal = -std::numeric_limits<double>::max();
        double val;
        for (int i=0; i<doNotCountLastSeconds; i++)
        {
            val = fabs(sumVec[i]);
            if (maxVal < sumVec[i])
                maxVal = sumVec[i];
        }
        if (maxVal != 0.0)
        {
            //multiplication is faster than division
            double maxValReziprocal = 1.0/maxVal;
            for (int i=0; i<doNotCountLastSeconds; i++)
            {
                sumVec[i] *= maxValReziprocal;
            }
        }
        
        loudnessMean = 0.0;
        loudnessRMS = 0.0;
        loudnessVariance = 0.0;
        loudnessRMSVariance = 0.0;
        
        //then calculate mean and root mean square
        for (int i=0; i<doNotCountLastSeconds; i++)
        {
            loudnessMean += sumVec[i];
            loudnessRMS += sumVec[i] * sumVec[i];
        }
        loudnessMean /= sumVec.size();
        loudnessRMS /= sumVec.size();
        loudnessRMS = std::sqrt(loudnessRMS);
        
        //then calculate variance
        double rmsVal;
        for (int i=0; i<doNotCountLastSeconds; i++)
        {
            val = sumVec[i] - loudnessMean;
            rmsVal = sumVec[i] - loudnessRMS;
            
            loudnessVariance    += val*val;
            loudnessRMSVariance += rmsVal*rmsVal;
        }
        loudnessVariance    /= sumVec.size();
        loudnessRMSVariance /= sumVec.size();
        
        loudnessMean = 1.0 - loudnessMean;
        loudnessRMS = 1.0 - loudnessRMS;
    }
    
    template class DynamicRangeCalculator<kiss_fft_scalar>;
}

#include "dynamic_range.hpp"

#include <cmath>
#include <limits>

namespace music
{
    DynamicRangeCalculator::DynamicRangeCalculator(PerTimeSliceStatistics* perTimeSliceStatistics) :
        ptss(perTimeSliceStatistics),
        loudnessMean(0.0),
        loudnessRMS(0.0),
        loudnessVariance(0.0),
        loudnessRMSVariance(0.0)
    {
        
    }

    void DynamicRangeCalculator::calculateDynamicRange()
    {
        ptss->calculateSum();
        
        Eigen::VectorXd sumVec = *(ptss->getSumVector());
        
        //first normalize our sum vector.
        double maxVal = -std::numeric_limits<double>::max();
        double val;
        for (int i=0; i<sumVec.size(); i++)
        {
            val = fabs(sumVec[i]);
            if (maxVal < sumVec[i])
                maxVal = sumVec[i];
        }
        if (maxVal != 0.0)
        {
            //multiplication is faster than division
            double maxValReziprocal = 1.0/maxVal;
            for (int i=0; i<sumVec.size(); i++)
            {
                sumVec[i] *= maxValReziprocal;
            }
        }
        
        loudnessMean = 0.0;
        loudnessRMS = 0.0;
        loudnessVariance = 0.0;
        loudnessRMSVariance = 0.0;
        
        //then calculate mean and root mean square
        for (int i=0; i<sumVec.size(); i++)
        {
            loudnessMean += sumVec[i];
            loudnessRMS += sumVec[i] * sumVec[i];
        }
        loudnessMean /= sumVec.size();
        loudnessRMS /= sumVec.size();
        loudnessRMS = std::sqrt(loudnessRMS);
        
        //then claculate variance
        double rmsVal;
        for (int i=0; i<sumVec.size(); i++)
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
}

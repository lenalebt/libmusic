#ifndef DYNAMIC_RANGE_HPP
#define DYNAMIC_RANGE_HPP

#include "feature_extraction_helper.hpp"

namespace music
{
    /**
     * @brief This class calculates the dynamic range of a normalized musical piece.
     * 
     * It first calculates a sum vector and then normalizes this vector.
     * After that, the mean and RMS are calculated, and the variances for these values.
     * 
     * @author Lena Brueder
     * @date 2012-07-06
     */
    class DynamicRangeCalculator
    {
    private:
        PerTimeSliceStatistics* ptss;
    protected:
        double loudnessMean;
        double loudnessRMS;          //RMS: root mean square
        double loudnessVariance;
        double loudnessRMSVariance;
    public:
        //Benutze das PerTimeSliceStatistics wegen dem Summenvektor, dann muss man den evtl nur einmal ausrechnen
        DynamicRangeCalculator(PerTimeSliceStatistics* perTimeSliceStatistics);
        
        double getLoudnessMean() const        {return loudnessMean;}
        double getLoudnessRMS() const         {return loudnessRMS;}
        double getLoudnessVariance() const    {return loudnessVariance;}
        double getLoudnessRMSVariance() const {return loudnessRMSVariance;}
        
        void calculateDynamicRange();
    };
}
#endif //DYNAMIC_RANGE_HPP

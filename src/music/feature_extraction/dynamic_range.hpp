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
     * RMS and the mean are in a range of [0,1].
     * To make the values more intuitive, they are inverted. Large values of
     * RMS and mean correspond to signals with a higher dynamic range.
     * 
     * @author Lena Brueder
     * @ingroup feature_extraction
     * @date 2012-07-06
     */
    template <typename ScalarType=kiss_fft_scalar>
    class DynamicRangeCalculator
    {
    private:
        PerTimeSliceStatistics<ScalarType>* ptss;
    protected:
        double loudnessMean;
        double loudnessRMS;          //RMS: root mean square
        double loudnessVariance;
        double loudnessRMSVariance;
    public:
        //Benutze das PerTimeSliceStatistics wegen dem Summenvektor, dann muss man den evtl nur einmal ausrechnen
        DynamicRangeCalculator(PerTimeSliceStatistics<ScalarType>* perTimeSliceStatistics);
        
        /**
         * @brief Returns the loudness mean.
         * @remarks This value has been inverted, such that higher values mean
         * higher dynamic range.
         */
        double getLoudnessMean() const        {return loudnessMean;}
        /**
         * @brief Returns the loudness RMS.
         * @remarks This value has been inverted, such that higher values mean
         * higher dynamic range.
         */
        double getLoudnessRMS() const         {return loudnessRMS;}
        /**
         * @brief Returns the variance when taking the loudness mean.
         * @remarks The variance has been calculated with the non-inverted mean
         * value.
         */
        double getLoudnessVariance() const    {return loudnessVariance;}
        /**
         * @brief Returns the variance when taking the loudness mean.
         * @remarks The variance has been calculated with the non-inverted RMS
         * value.
         */
        double getLoudnessRMSVariance() const {return loudnessRMSVariance;}
        
        /**
         * @brief Calculates the values that can be retrieved through this
         *      class.
         * 
         * If this function is not called before trying to read the values,
         * the results are undefined.
         */
        void calculateDynamicRange();
    };
}
#endif //DYNAMIC_RANGE_HPP

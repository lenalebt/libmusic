#ifndef FEATURE_EXTRACTION_HELPER_HPP
#define FEATURE_EXTRACTION_HELPER_HPP

#include "constantq.hpp"
#include <Eigen/Dense>

namespace music
{
    /**
     * @defgroup feature_extraction Feature Extraction
     * @brief All classes in this module belong to feature extraction.
     */
    
    /**
     * @brief This class creates statistics per bin of a constant Q transform result.
     * 
     * The class is optimized for computation speed, so it tries to calculate some
     * values at once. this may lead to some waste of memory, but is faster if
     * you need some information at once.
     * 
     * @see PerTimeSliceStatistics
     * @author Lena Brueder
     * @ingroup feature_extraction
     * @date 2012-07-02
     */
    template <typename ScalarType=kiss_fft_scalar>
    class PerBinStatistics
    {
    private:
        ConstantQTransformResult* transformResult;
        
    protected:
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* meanVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* varianceVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* minVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* maxVector;
    public:
        PerBinStatistics(ConstantQTransformResult* transformResult);
        ~PerBinStatistics();
        
        /**
         * @brief Calculates the mean, maximum and minimum values per bin.
         * 
         * It is faster to calculate all these things at once, since you
         * only need to read the memory once for all features.
         */
        void calculateMeanMinMax();
        /**
         * @brief Calculates the variance of all bins.
         * 
         * Since one needs the mean to calculates the variance, this is
         * not done in the same step.
         */
        void calculateVariance();
        
        /**
         * @brief Returns the mean vector with the means per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the mean vector with the means per time slice
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getMeanVector() const            {return meanVector;}
        /**
         * @brief Returns the max vector with the maximum values per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the max vector with the maximum values per time slice.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getMaxVector() const             {return maxVector;}
        /**
         * @brief Returns the min vector with the minimum per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the min vector with the minimums per time slice
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getMinVector() const             {return minVector;}
        /**
         * @brief Returns the variance vector with the variances per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the variance vector with the variances per time slice
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getVarianceVector() const        {return varianceVector;}
    };
    
    /**
     * @brief This class creates statistics per time slice of a constant Q transform result.
     * 
     * The class is optimized for computation speed, so it tries to calculate some
     * values at once. this may lead to some waste of memory, but is faster if
     * you need some information at once.
     * 
     * @ingroup feature_extraction
     * @see PerBinStatistics
     * @author Lena Brueder
     * @date 2012-07-02
     */
    template <typename ScalarType=kiss_fft_scalar>
    class PerTimeSliceStatistics
    {
    private:
        ConstantQTransformResult* transformResult;
        
    protected:
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* meanVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* varianceVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* minVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* maxVector;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* sumVector;
        
        double timeResolution;
    public:
        PerTimeSliceStatistics(ConstantQTransformResult* transformResult, double timeResolution=0.005);
        ~PerTimeSliceStatistics();
        
        double getTimeResolution()      {return timeResolution;}
        
        void calculateSum();
        /**
         * @brief Calculates the mean, maximum, minimum and sum values per bin.
         * 
         * It is faster to calculate all these things at once, since you
         * only need to read the memory once for all features.
         * 
         * The sum effectively is the same as the mean, there is only a constant
         * factor that differentiates them. If you want to calculate the sum, too,
         * you need to explicitly request the calculation. It is faster to do the
         * calculation in one step than in an extra function, so it is faster
         * to request it here than to calculate it later on via calculateSum().
         */
        void calculateMeanMinMaxSum(bool calculateSum = false);
        /**
         * @brief Calculates the variance.
         * 
         * @remarks Will calculate mean, if needed. If you need the sum, too, it
         *      is faster to first call calculateMeanMinMaxSum() and then call
         *      calculateVariance().
         * 
         */
        void calculateVariance();
        
        /**
         * @brief Returns the mean vector with the means per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * @attention You need to first calculate the values you need, see linked functions.
         * 
         * @see calculateMeanMinMaxSum
         * 
         * @return the mean vector with the means per time slice, or <code>NULL</code> if it has not yet been calculated
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getMeanVector() const            {return meanVector;}
        /**
         * @brief Returns the max vector with the maximum values per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * @attention You need to first calculate the values you need, see linked functions.
         * 
         * @see calculateMeanMinMaxSum
         * 
         * @return the max vector with the maximum values per time slice, or <code>NULL</code> if it has not yet been calculated
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getMaxVector() const             {return maxVector;}
        /**
         * @brief Returns the min vector with the minimum per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * @attention You need to first calculate the values you need, see linked functions.
         * 
         * @see calculateMeanMinMaxSum
         * 
         * @return the min vector with the minimums per time slice, or <code>NULL</code> if it has not yet been calculated
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getMinVector() const             {return minVector;}
        /**
         * @brief Returns the variance vector with the variances per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * @attention You need to first calculate the values you need, see linked functions.
         * 
         * @see calculateVariance
         * 
         * @return the variance vector with the variances per time slice, or <code>NULL</code> if it has not yet been calculated
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getVarianceVector() const        {return varianceVector;}
        /**
         * @brief Returns the sum vector with the sums per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * @attention You need to first calculate the values you need, see linked functions.
         * 
         * @see calculateSum
         * @see calculateMeanMinMaxSum
         * 
         * @return the sum vector with the sums per time slice, or <code>NULL</code> if it has not yet been calculated
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>* getSumVector() const             {return sumVector;}
    };
}
#endif //FEATURE_EXTRACTION_HELPER_HPP

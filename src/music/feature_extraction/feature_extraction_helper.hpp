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
     * @author Lena Brueder
     * @ingroup feature_extraction
     * @date 2012-07-02
     */
    class PerBinStatistics
    {
    private:
        ConstantQTransformResult* transformResult;
        
    protected:
        Eigen::VectorXd* meanVector;
        Eigen::VectorXd* varianceVector;
        Eigen::VectorXd* minVector;
        Eigen::VectorXd* maxVector;
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
        const Eigen::VectorXd* getMeanVector() const            {return meanVector;}
        /**
         * @brief Returns the max vector with the maximum values per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the max vector with the maximum values per time slice.
         */
        const Eigen::VectorXd* getMaxVector() const             {return maxVector;}
        /**
         * @brief Returns the min vector with the minimum per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the min vector with the minimums per time slice
         */
        const Eigen::VectorXd* getMinVector() const             {return minVector;}
        /**
         * @brief Returns the variance vector with the variances per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the variance vector with the variances per time slice
         */
        const Eigen::VectorXd* getVarianceVector() const        {return varianceVector;}
    };
    
    /**
     * @brief This class creates statistics per time slice of a constant Q transform result.
     * 
     * The class is optimized for computation speed, so it tries to calculate some
     * values at once. this may lead to some waste of memory, but is faster if
     * you need some information at once.
     * 
     * @author Lena Brueder
     * @date 2012-07-02
     */
    class PerTimeSliceStatistics
    {
    private:
        ConstantQTransformResult* transformResult;
        
    protected:
        Eigen::VectorXd* meanVector;
        Eigen::VectorXd* varianceVector;
        Eigen::VectorXd* minVector;
        Eigen::VectorXd* maxVector;
        Eigen::VectorXd* sumVector;
        
        double timeResolution;
    public:
        PerTimeSliceStatistics(ConstantQTransformResult* transformResult, double timeResolution=0.005);
        ~PerTimeSliceStatistics();
        
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
         * @brief Calculates the variance
         * 
         * 
         * 
         * @return 
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
        const Eigen::VectorXd* getMeanVector() const            {return meanVector;}
        /**
         * @brief Returns the max vector with the maximum values per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the max vector with the maximum values per time slice.
         */
        const Eigen::VectorXd* getMaxVector() const             {return maxVector;}
        /**
         * @brief Returns the min vector with the minimum per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the min vector with the minimums per time slice
         */
        const Eigen::VectorXd* getMinVector() const             {return minVector;}
        /**
         * @brief Returns the variance vector with the variances per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the variance vector with the variances per time slice
         */
        const Eigen::VectorXd* getVarianceVector() const        {return varianceVector;}
        /**
         * @brief Returns the sum vector with the sums per time slice.
         * 
         * @remarks Do not delete the pointer returned by this function.
         *      The class will delete the pointer upon destruction.
         * 
         * @return the sum vector with the sums per time slice
         */
        const Eigen::VectorXd* getSumVector() const             {return sumVector;}
    };
}
#endif //FEATURE_EXTRACTION_HELPER_HPP

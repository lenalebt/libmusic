#ifndef FEATURE_EXTRACTION_HELPER_HPP
#define FEATURE_EXTRACTION_HELPER_HPP

#include "constantq.hpp"
#include <Eigen/Dense>

namespace music
{
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
        
        void calculateMeanMinMax();
        void calculateVariance();
        
        const Eigen::VectorXd* getMeanVector() const            {return meanVector;}
        const Eigen::VectorXd* getMaxVector() const             {return maxVector;}
        const Eigen::VectorXd* getMinVector() const             {return minVector;}
        const Eigen::VectorXd* getVarianceVector() const        {return varianceVector;}
    };
    
    class PerTimeSliceStatistics
    {
    private:
        ConstantQTransformResult* transformResult;
        
    protected:
        Eigen::VectorXd* meanVector;
        Eigen::VectorXd* varianceVector;
        Eigen::VectorXd* minVector;
        Eigen::VectorXd* maxVector;
        
        double timeResolution;
    public:
        PerTimeSliceStatistics(ConstantQTransformResult* transformResult, double timeResolution);
        
        void calculateMeanMinMax();
        void calculateVariance();
        
        const Eigen::VectorXd* getMeanVector() const            {return meanVector;}
        const Eigen::VectorXd* getMaxVector() const             {return maxVector;}
        const Eigen::VectorXd* getMinVector() const             {return minVector;}
        const Eigen::VectorXd* getVarianceVector() const        {return varianceVector;}
    };
}
#endif //FEATURE_EXTRACTION_HELPER_HPP

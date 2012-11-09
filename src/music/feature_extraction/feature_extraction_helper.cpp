#include "feature_extraction_helper.hpp"

#include <assert.h>
#include <limits>

namespace music
{
    PerBinStatistics::PerBinStatistics(ConstantQTransformResult* transformResult) :
        transformResult(transformResult),
        meanVector(NULL),
        varianceVector(NULL),
        minVector(NULL),
        maxVector(NULL)
    {
        assert(transformResult != NULL);
    }
    
    void PerBinStatistics::calculateMeanMinMax()
    {
        meanVector = new Eigen::VectorXd(transformResult->getOctaveCount() * transformResult->getBinsPerOctave());
        minVector = new Eigen::VectorXd(transformResult->getOctaveCount() * transformResult->getBinsPerOctave());
        maxVector = new Eigen::VectorXd(transformResult->getOctaveCount() * transformResult->getBinsPerOctave());
        
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            const Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix = transformResult->getOctaveMatrix(octave);
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                double mean = 0.0;
                double min = std::numeric_limits<double>::max();
                double max = -std::numeric_limits<double>::max();
                for (int i=0; i<octaveMatrix->cols(); i++)
                {
                    double val = std::abs((*octaveMatrix)(bin, i));
                    
                    mean += val;
                    
                    if (val < min)
                        min = val;
                    
                    if (val > max)
                        max = val;
                }
                mean /= octaveMatrix->cols();
                
                int pos = octave*transformResult->getBinsPerOctave() + bin;
                (*meanVector)(pos) = mean;
                (*minVector)(pos) = min;
                (*maxVector)(pos) = max;
            }
        }
    }
    
    void PerBinStatistics::calculateVariance()
    {
        if (meanVector == NULL)
            calculateMeanMinMax();
        
        varianceVector = new Eigen::VectorXd(transformResult->getOctaveCount() * transformResult->getBinsPerOctave());
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            const Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix = transformResult->getOctaveMatrix(octave);
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                int pos = octave*transformResult->getBinsPerOctave() + bin;
                
                double mean = (*meanVector)(pos);
                double variance = 0.0;
                for (int i=0; i<octaveMatrix->cols(); i++)
                {
                    double val = std::abs((*octaveMatrix)(bin, i)) - mean;
                    
                    variance += val*val;
                }
                variance /= octaveMatrix->cols();
                
                
                (*varianceVector)(pos) = variance;
            }
        }
    }
}

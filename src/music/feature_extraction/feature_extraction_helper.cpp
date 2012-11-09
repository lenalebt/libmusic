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
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        meanVector = new Eigen::VectorXd(octaveCount * binsPerOctave);
        minVector = new Eigen::VectorXd(octaveCount * binsPerOctave);
        maxVector = new Eigen::VectorXd(octaveCount * binsPerOctave);
        
        for (int octave=0; octave<octaveCount; octave++)
        {
            const Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix = transformResult->getOctaveMatrix(octave);
            
            Eigen::VectorXd mean(binsPerOctave);
            Eigen::VectorXd min(binsPerOctave);
            Eigen::VectorXd max(binsPerOctave);
            
            for (int bin=0; bin<binsPerOctave; bin++)
            {
                mean[bin] = 0.0;
                min[bin] = std::numeric_limits<double>::max();
                max[bin] = -std::numeric_limits<double>::max();
            }
            
            //matricies are column-major, so summing this way is faster than summing the other way round
            for (int i=0; i<octaveMatrix->cols(); i++)
            {
                
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    double val = std::abs((*octaveMatrix)(bin, i));
                    
                    mean[bin] += val;
                    
                    if (val < min[bin])
                        min[bin] = val;
                    
                    if (val > max[bin])
                        max[bin] = val;
                }
            }
            
            mean /= octaveMatrix->cols();
            for (int bin=0; bin<binsPerOctave; bin++)
            {
                int pos = octave*binsPerOctave + bin;
                (*meanVector)(pos) = mean[bin];
                (*minVector)(pos) = min[bin];
                (*maxVector)(pos) = max[bin];
            }
        }
    }
    
    void PerBinStatistics::calculateVariance()
    {
        if (meanVector == NULL)
            calculateMeanMinMax();
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        varianceVector = new Eigen::VectorXd(octaveCount * binsPerOctave);
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            const Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix = transformResult->getOctaveMatrix(octave);
            
            Eigen::VectorXd variance(binsPerOctave);
            
            for (int bin=0; bin<binsPerOctave; bin++)
                variance[bin] = 0.0;
            
            //matricies are column-major, so summing this way is faster than summing the other way round
            for (int i=0; i<octaveMatrix->cols(); i++)
            {
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    double val = std::abs((*octaveMatrix)(bin, i)) - (*meanVector)[octave*binsPerOctave + bin];
                    
                    variance[bin] += val*val;
                }
            }
            
            variance /= octaveMatrix->cols();
            for (int bin=0; bin<binsPerOctave; bin++)
            {
                int pos = octave*binsPerOctave + bin;
                (*varianceVector)(pos) = variance[bin];
            }
        }
    }
    
    PerTimeSliceStatistics::PerTimeSliceStatistics(ConstantQTransformResult* transformResult, double timeResolution) :
        transformResult(transformResult),
        meanVector(NULL),
        varianceVector(NULL),
        minVector(NULL),
        maxVector(NULL),
        
        timeResolution(timeResolution)
    {
        assert(transformResult != NULL);
        assert(timeResolution > 0.0);
    }
    
    void PerTimeSliceStatistics::calculateMeanMinMax()
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
    
    void PerTimeSliceStatistics::calculateVariance()
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

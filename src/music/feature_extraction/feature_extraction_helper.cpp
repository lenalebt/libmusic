#include "feature_extraction_helper.hpp"

#include <assert.h>
#include <limits>

#include "debug.hpp"

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
        sumVector(NULL),
        
        timeResolution(timeResolution)
    {
        assert(transformResult != NULL);
        assert(timeResolution > 0.0);
    }
    
    void PerTimeSliceStatistics::calculateSum()
    {
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        int elementCount = transformResult->getOriginalDuration() / timeResolution;
        sumVector = new Eigen::VectorXd(elementCount);
        
        if (meanVector == NULL)
        {
            for (int i=0; i < elementCount; i++)
            {
                double sum=0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    for (int bin=0; bin<binsPerOctave; bin++)
                    {
                        sum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                    }
                }
                (*sumVector)[i] = sum;
            }
        }
        else
        {
            /* 
             * If we already calculated the mean Vector, we can get the sum vector
             * without summing again, so this should be faster. It might
             * not be that accurate, but it should not be a problem here.
             */
            (*sumVector) = (*meanVector) * (binsPerOctave * octaveCount);
        }
    }
    void PerTimeSliceStatistics::calculateMeanMinMaxSum(bool calculateSum)
    {
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        int elementCount = transformResult->getOriginalDuration() / timeResolution;
        meanVector = new Eigen::VectorXd(elementCount);
        minVector = new Eigen::VectorXd(elementCount);
        maxVector = new Eigen::VectorXd(elementCount);
        
        for (int i=0; i<elementCount; i++)
        {
            double mean = 0.0;
            double min = std::numeric_limits<double>::max();
            double max = -std::numeric_limits<double>::max();
            for (int octave=0; octave<octaveCount; octave++)
            {
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    double val = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                    mean += val;
                    
                    if (val < min)
                        min = val;
                    
                    if (val > max)
                        max = val;
                }
            }
            (*meanVector)(i) = mean;
            (*minVector)(i) = min;
            (*maxVector)(i) = max;
        }
        
        if (calculateSum && (sumVector == NULL))
        {
            sumVector = new Eigen::VectorXd(elementCount);
            *sumVector = *meanVector;
        }
        
        (*meanVector) /= binsPerOctave * octaveCount;
    }
    
    void PerTimeSliceStatistics::calculateVariance()
    {
        if (meanVector == NULL)
            calculateMeanMinMaxSum(false);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        int elementCount = transformResult->getOriginalDuration() / timeResolution;
        varianceVector = new Eigen::VectorXd(elementCount);
        for (int i=0; i<elementCount; i++)
        {
            double variance = 0.0;
            for (int octave=0; octave<octaveCount; octave++)
            {
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    double val = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin)) - (*meanVector)[octave*binsPerOctave + bin];
                    variance += val*val;
                }
            }
            (*varianceVector)(i) = variance;
        }
        (*varianceVector) /= binsPerOctave * octaveCount;
    }
}

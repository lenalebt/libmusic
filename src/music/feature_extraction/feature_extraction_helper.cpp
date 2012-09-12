#include "feature_extraction_helper.hpp"

#include <assert.h>
#include <limits>

#include "debug.hpp"

namespace music
{
    template <typename ScalarType>
    PerBinStatistics<ScalarType>::PerBinStatistics(ConstantQTransformResult* transformResult) :
        transformResult(transformResult),
        meanVector(NULL),
        varianceVector(NULL),
        minVector(NULL),
        maxVector(NULL)
    {
        assert(transformResult != NULL);
    }
    template <typename ScalarType>
    PerBinStatistics<ScalarType>::~PerBinStatistics()
    {
        if (meanVector)
            delete meanVector;
        if (varianceVector)
            delete varianceVector;
        if (minVector)
            delete minVector;
        if (maxVector)
            delete maxVector;
    }
    template <typename ScalarType>
    void PerBinStatistics<ScalarType>::calculateMeanMinMax()
    {
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        meanVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(octaveCount * binsPerOctave);
        minVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(octaveCount * binsPerOctave);
        maxVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(octaveCount * binsPerOctave);
        
        for (int octave=0; octave<octaveCount; octave++)
        {
            const Eigen::Matrix<std::complex<ScalarType>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix = transformResult->getOctaveMatrix(octave);
            
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean(binsPerOctave);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> min(binsPerOctave);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> max(binsPerOctave);
            
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
    
    template <typename ScalarType>
    void PerBinStatistics<ScalarType>::calculateVariance()
    {
        if (meanVector == NULL)
            calculateMeanMinMax();
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        varianceVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(octaveCount * binsPerOctave);
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            const Eigen::Matrix<std::complex<ScalarType>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix = transformResult->getOctaveMatrix(octave);
            
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> variance(binsPerOctave);
            
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
    
    template <typename ScalarType>
    PerTimeSliceStatistics<ScalarType>::PerTimeSliceStatistics(ConstantQTransformResult* transformResult, double timeResolution) :
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
    
    template <typename ScalarType>
    PerTimeSliceStatistics<ScalarType>::~PerTimeSliceStatistics()
    {
        if (meanVector)
            delete meanVector;
        if (varianceVector)
            delete varianceVector;
        if (minVector)
            delete minVector;
        if (maxVector)
            delete maxVector;
        if (sumVector)
            delete sumVector;
    }
    
    template <typename ScalarType>
    void PerTimeSliceStatistics<ScalarType>::calculateSum()
    {
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        int elementCount = transformResult->getOriginalDuration() / timeResolution;
        sumVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(elementCount);
        
        if (meanVector == NULL)
        {
            for (int i=0; i < elementCount; i++)
            {
                double sum=0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    for (int bin=0; bin<binsPerOctave; bin++)
                    {
                        sum += transformResult->getNoteValueMean(i*timeResolution, octave, bin, timeResolution);
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
    
    template <typename ScalarType>
    void PerTimeSliceStatistics<ScalarType>::calculateMeanMinMaxSum(bool calculateSum)
    {
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        int elementCount = transformResult->getOriginalDuration() / timeResolution;
        meanVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(elementCount);
        minVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(elementCount);
        maxVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(elementCount);
        
        for (int i=0; i<elementCount; i++)
        {
            double mean = 0.0;
            double min = std::numeric_limits<double>::max();
            double max = -std::numeric_limits<double>::max();
            for (int octave=0; octave<octaveCount; octave++)
            {
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    double val = transformResult->getNoteValueMean(i*timeResolution, octave, bin, timeResolution);
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
            sumVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(elementCount);
            *sumVector = *meanVector;
        }
        
        (*meanVector) /= binsPerOctave * octaveCount;
    }
    
    template <typename ScalarType>
    void PerTimeSliceStatistics<ScalarType>::calculateVariance()
    {
        if (meanVector == NULL)
            calculateMeanMinMaxSum(false);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        int elementCount = transformResult->getOriginalDuration() / timeResolution;
        varianceVector = new Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(elementCount);
        for (int i=0; i<elementCount; i++)
        {
            double variance = 0.0;
            for (int octave=0; octave<octaveCount; octave++)
            {
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    double val = transformResult->getNoteValueMean(i*timeResolution, octave, bin, timeResolution) - (*meanVector)[octave*binsPerOctave + bin];
                    variance += val*val;
                }
            }
            (*varianceVector)(i) = variance;
        }
        (*varianceVector) /= binsPerOctave * octaveCount;
    }
    
    template class PerBinStatistics<kiss_fft_scalar>;
    template class PerTimeSliceStatistics<kiss_fft_scalar>;
}

#include "bpm.hpp"
#include "debug.hpp"

#include "feature_extraction_helper.hpp"

#include <fstream>
#include <algorithm>

namespace music
{
    template <typename T> int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
    
    void BPMEstimator::estimateBPM(ConstantQTransformResult* transformResult)
    {
        estimateBPM1(transformResult);
    }
    
    void BPMEstimator::estimateBPM1(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/100.0;
        
        PerTimeSliceStatistics timeSliceStatistics(transformResult, timeSliceLength);
        timeSliceStatistics.calculateSum(/*0.0, transformResult->getOriginalDuration()*/);
        
        assert(timeSliceStatistics.getSumVector() != NULL);
        
        int maxDuration = timeSliceStatistics.getSumVector()->size();
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr2("sumVec.dat");
            for (int i=1; i<timeSliceStatistics.getSumVector()->size(); i++)
            {
                outstr2 << (*timeSliceStatistics.getSumVector())[i];
                outstr2 << std::endl;
            }
        #endif
        
        DEBUG_OUT("building derivation of sum vector...", 15);
        //6 seconds maximum shift
        int maxCorrShift=std::min(int(6.0/timeSliceLength), maxDuration-1);
        Eigen::VectorXf derivSum(maxDuration-1);
        for (int shift=0; shift<maxDuration-1; shift++)
        {
            derivSum[shift] = (*timeSliceStatistics.getSumVector())[shift+1] - (*timeSliceStatistics.getSumVector())[shift];
        }
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr3("derivSum.dat");
            for (int i=1; i<derivSum.size(); i++)
            {
                outstr3 << derivSum[i];
                outstr3 << std::endl;
            }
        #endif
        
        
        DEBUG_OUT("calculating auto correlation of derivation vector...", 15);
        Eigen::VectorXf autoCorr(maxCorrShift);
        for (int shift=0; shift<maxCorrShift; shift++)
        {
            double corr=0.0;
            for (int i=0; i<derivSum.size(); i++)
            {
                int shiftPos = i + shift;
                corr += derivSum[i] * ((shiftPos < derivSum.size()) ? derivSum[shiftPos] : 0.0);
            }
            autoCorr[shift] = corr;
        }
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr4("autoCorr.dat");
            for (int i=1; i<autoCorr.size(); i++)
            {
                outstr4 << autoCorr[i];
                outstr4 << std::endl;
            }
        #endif
        
        
        DEBUG_OUT("looking for maxima...", 15);
        std::vector<int> maxCorrPos;
        {
            DEBUG_OUT("calculate mean...", 20);
            double mean=0.0;
            for (int i=0; i<autoCorr.size(); i++)
            {
                mean += autoCorr[i];
            }
            mean /= autoCorr.size();
            
            DEBUG_OUT("calculate variance...", 20);
            double variance=0.0;
            for (int i=0; i<autoCorr.size(); i++)
            {
                double val = autoCorr[i] - mean;
                variance += val*val;
            }
            variance /= autoCorr.size();
            double standardDerivation = sqrt(variance);
            
            DEBUG_OUT("build barrier breaker list...", 20);
            double barrierVal = mean + standardDerivation;
            int lastAddedPosition = -10000;
            for (int i=0; i<autoCorr.size(); i++)
            {
                if ((i-lastAddedPosition) > 0.2/timeSliceLength)
                {
                    if (autoCorr[i] > barrierVal)
                        maxCorrPos.push_back(lastAddedPosition=i);
                }
            }
        }
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<int> diffPosVector;
        {
            double bpmMean = 0.0;
            int oldVal = *maxCorrPos.begin();
            for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                int val = *it - oldVal;
                diffPosVector.push_back(val);
                DEBUG_OUT("beat length bpm: " << 60.0/(double(val)*timeSliceLength), 30);
                bpmMean += val;
                
                oldVal = *it;
            }
            this->bpmMean = 60.0/(bpmMean / diffPosVector.size()*timeSliceLength);
            //sorting vector to get the median
            std::sort(diffPosVector.begin(), diffPosVector.end());
        }
        
        while (this->bpmMean > 250) //try to get rid of too fast guesses
            this->bpmMean /= 2.0;
        
        this->bpmMedian = 60.0/(double(diffPosVector[diffPosVector.size()/2])*timeSliceLength);
        
        this->bpmVariance = 0.0;
        for (std::vector<int>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceLength) - bpmMean;
            this->bpmVariance += val*val;
        }
        this->bpmVariance /= diffPosVector.size();
    }
    
    void BPMEstimator::estimateBPM2(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/200.0;
        
        PerTimeSliceStatistics timeSliceStatistics(transformResult, timeSliceLength);
        timeSliceStatistics.calculateSum(/*0.0, transformResult->getOriginalDuration()*/);
        
        assert(timeSliceStatistics.getSumVector() != NULL);
        
        int maxDuration = timeSliceStatistics.getSumVector()->size();
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr2("sumVec.dat");
            for (int i=1; i<timeSliceStatistics.getSumVector()->size(); i++)
            {
                outstr2 << (*timeSliceStatistics.getSumVector())[i];
                outstr2 << std::endl;
            }
        #endif
        
        DEBUG_OUT("building derivation of sum vector...", 15);
        //6 seconds maximum shift
        int maxCorrShift=std::min(int(6.0/timeSliceLength), maxDuration-1);
        Eigen::VectorXf derivSum(maxDuration-1);
        for (int shift=0; shift<maxDuration-1; shift++)
        {
            derivSum[shift] = (*timeSliceStatistics.getSumVector())[shift+1] - (*timeSliceStatistics.getSumVector())[shift];
        }
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr3("derivSum.dat");
            for (int i=1; i<derivSum.size(); i++)
            {
                outstr3 << derivSum[i];
                outstr3 << std::endl;
            }
        #endif
        
        
        DEBUG_OUT("calculating auto correlation of derivation vector...", 15);
        Eigen::VectorXf autoCorr(maxCorrShift);
        for (int shift=0; shift<maxCorrShift; shift++)
        {
            double corr=0.0;
            for (int i=0; i<derivSum.size(); i++)
            {
                int shiftPos = i + shift;
                corr += derivSum[i] * ((shiftPos < derivSum.size()) ? derivSum[shiftPos] : 0.0);
            }
            autoCorr[shift] = corr;
        }
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr4("autoCorr.dat");
            for (int i=1; i<autoCorr.size(); i++)
            {
                outstr4 << autoCorr[i];
                outstr4 << std::endl;
            }
        #endif
        
        
        DEBUG_OUT("looking for maxima...", 15);
        std::vector<int> maxCorrPos;
        {
            DEBUG_OUT("calculate mean...", 20);
            double mean=0.0;
            for (int i=0; i<autoCorr.size(); i++)
            {
                mean += autoCorr[i];
            }
            mean /= autoCorr.size();
            
            DEBUG_OUT("calculate variance...", 20);
            double variance=0.0;
            for (int i=0; i<autoCorr.size(); i++)
            {
                double val = autoCorr[i] - mean;
                variance += val*val;
            }
            variance /= autoCorr.size();
            double standardDerivation = sqrt(variance);
            
            DEBUG_OUT("build barrier breaker list...", 20);
            double barrierVal = mean + standardDerivation;
            int lastAddedPosition = -10000;
            double tmp[4] = {0.0, 0.0, 0.0, 0.0};
            for (int i=0; i<autoCorr.size(); i++)
            {
                if ((i-lastAddedPosition) > 0.2/timeSliceLength)
                {
                    if (autoCorr[i] > barrierVal)
                    {
                        for (int j=1; (j<0.2/timeSliceLength) && (autoCorr.size() < i+j); j++)
                        {
                            if (autoCorr[i+j] < barrierVal)
                            {
                                i+=j/2;
                                break;
                            }
                        }
                        maxCorrPos.push_back(lastAddedPosition=i);
                        
                        int num = maxCorrPos.size();
                        tmp[0] += autoCorr[i];
                        if (num % 2)
                            tmp[1] += autoCorr[i];
                        if (num % 3)
                            tmp[2] += autoCorr[i];
                        if (num % 4)
                            tmp[3] += autoCorr[i];
                    }
                }
            }
            DEBUG_OUT("tmp[" << 0 << "]=" << tmp[0] << "...", 20);
            DEBUG_OUT("tmp[" << 1 << "]=" << tmp[1] << "...", 20);
            DEBUG_OUT("tmp[" << 2 << "]=" << tmp[2] << "...", 20);
            DEBUG_OUT("tmp[" << 3 << "]=" << tmp[3] << "...", 20);
        }
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<int> diffPosVector;
        {
            double bpmMean = 0.0;
            int oldVal = *maxCorrPos.begin();
            for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                int val = *it - oldVal;
                diffPosVector.push_back(val);
                DEBUG_OUT("beat length bpm: " << 60.0/(double(val)*timeSliceLength), 30);
                bpmMean += val;
                
                oldVal = *it;
            }
            this->bpmMean = 60.0/(bpmMean / diffPosVector.size()*timeSliceLength);
            //sorting vector to get the median
            std::sort(diffPosVector.begin(), diffPosVector.end());
        }
        
        while (this->bpmMean > 250) //try to get rid of too fast guesses
            this->bpmMean /= 2.0;
        
        this->bpmMedian = 60.0/(double(diffPosVector[diffPosVector.size()/2])*timeSliceLength);
        
        this->bpmVariance = 0.0;
        for (std::vector<int>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceLength) - bpmMean;
            this->bpmVariance += val*val;
        }
        this->bpmVariance /= diffPosVector.size();
    }
    
}

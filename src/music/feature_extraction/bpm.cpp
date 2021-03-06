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
    
    template <typename ScalarType>
    bool BPMEstimator<ScalarType>::estimateBPM(PerTimeSliceStatistics<ScalarType>* timeSliceStatistics)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        timeSliceStatistics->calculateSum(/*0.0, transformResult->getOriginalDuration()*/);
        
        assert(timeSliceStatistics->getSumVector() != NULL);
        int maxDuration = timeSliceStatistics->getSumVector()->size();
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> sumVec = *(timeSliceStatistics->getSumVector());
        
        //TODO: low-pass-filter sumVec (moving average, 5 values)
        double hist[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int histPos=0;
        for (int i=0; i<sumVec.size(); i++)
        {
            hist[histPos]=sumVec[i];
            
            sumVec[i] = (hist[0] + hist[1] + hist[2] + hist[3] + hist[4]) / 5.0;
            //sumVec[i] = (hist[0] + hist[1] + hist[2] + hist[3] + hist[4] + hist[5] + hist[6] + hist[7] + hist[8] + hist[9]) / 10.0;
            
            histPos = (histPos + 1) % 5;
            //histPos = (histPos + 1) % 10;
        }
        
        #if DEBUG_LEVEL>=25
            std::ofstream outstr2("sumVec.dat");
            for (int i=1; i<sumVec.size(); i++)
            {
                outstr2 << sumVec[i];
                outstr2 << std::endl;
            }
        #endif
        
        DEBUG_OUT("building derivation of sum vector...", 15);
        //6 seconds maximum shift
        int maxCorrShift=std::min(int(6.0/timeSliceStatistics->getTimeResolution()), maxDuration-1);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> derivSum(maxDuration-1);
        for (int shift=0; shift<maxDuration-1; shift++)
        {
            derivSum[shift] = sumVec[shift+1] - sumVec[shift];
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
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> autoCorr(maxCorrShift);
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
        std::vector<float> maxCorrPos;
        {
            DEBUG_OUT("calculate mean...", 20);
            double mean=0.0;
            double maxVal = -std::numeric_limits<double>::max();
            for (int i=0; i<autoCorr.size(); i++)
            {
                mean += autoCorr[i];
                if ((i>20) && (autoCorr[i] > maxVal))
                    maxVal = autoCorr[i];
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
            barrierVal = (barrierVal > 0.8*maxVal) ? 0.8*maxVal : barrierVal;
            DEBUG_OUT("barrier value: " << barrierVal, 30);
            int lastAddedPosition = -10000;
            int risePos=-1;
            for (int i=1; i<autoCorr.size()-1; i++)
            {
                if ((i-lastAddedPosition) > 0.2/timeSliceStatistics->getTimeResolution())
                {
                    if ((risePos < 0) && (autoCorr[i] > barrierVal) && (autoCorr[i] - autoCorr[i-1] > 0))   //rise
                    {
                        DEBUG_OUT("rise ", 40);
                        risePos = i;
                    }
                    else if ((risePos >= 0) && (autoCorr[i] < barrierVal) && (autoCorr[i+1] - autoCorr[i] < 0))   //fall
                    {
                        DEBUG_OUT("fall ", 40);
                        maxCorrPos.push_back(lastAddedPosition = risePos+(float(i-risePos)/2.0));
                        DEBUG_OUT(lastAddedPosition, 40);
                        risePos=-1;
                    }
                }
            }
        }
        
        if (maxCorrPos.size() <= 1)
            return false;
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<float> diffPosVector;
        {
            double bpmMean = 0.0;
            float oldVal = *maxCorrPos.begin();
            for (std::vector<float>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                float val = *it - oldVal;
                diffPosVector.push_back(val);
                DEBUG_OUT("beat length bpm: " << 60.0/(double(val)*timeSliceStatistics->getTimeResolution()), 30);
                DEBUG_OUT("val: " << double(val), 30);
                bpmMean += val;
                
                oldVal = *it;
            }
            this->bpmMean = 60.0/(bpmMean / diffPosVector.size()*timeSliceStatistics->getTimeResolution());
            //sorting vector to get the median
            std::sort(diffPosVector.begin(), diffPosVector.end());
        }
        
        if (diffPosVector.size() == 0)
            return false;
        
        while (this->bpmMean > 250) //try to get rid of too fast guesses
            this->bpmMean /= 2.0;
        
        this->bpmMedian = 60.0/(double(diffPosVector[diffPosVector.size()/2])*timeSliceStatistics->getTimeResolution());
        
        this->bpmVariance = 0.0;
        for (std::vector<float>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceStatistics->getTimeResolution()) - bpmMean;
            this->bpmVariance += val*val;
        }
        this->bpmVariance /= diffPosVector.size();
        
        DEBUG_OUT("cancel out outliers...", 15);
        double stdDeriv = sqrt(this->bpmVariance);
        double newMean = 0.0;
        std::vector<int> newDiffPosVector;
 //       double newVariance = 0.0;
        for (std::vector<float>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceStatistics->getTimeResolution()) - bpmMean;
            DEBUG_OUT(val << "; " << stdDeriv, 40);
            if (fabs(val) > stdDeriv + 5)
            {
                DEBUG_OUT("cancel out " << *it, 20);
            }
            else
            {
                newMean += *it;
                newDiffPosVector.push_back(*it);
            }
        }
        this->bpmMean = 60.0/(newMean / newDiffPosVector.size()*timeSliceStatistics->getTimeResolution());
        
        return true;
    }
    
    template class BPMEstimator<kiss_fft_scalar>;
}

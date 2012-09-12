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
    void BPMEstimator<ScalarType>::estimateBPM(ConstantQTransformResult* transformResult)
    {
        estimateBPM1(transformResult);
        //estimateBPM5(transformResult);
    }
    
    template <typename ScalarType>
    void BPMEstimator<ScalarType>::estimateBPM5(ConstantQTransformResult* transformResult)
    {
        //method from "Joint Beat&Tatum Tracking from Music Signals"
        
    }
    template <typename ScalarType>
    void BPMEstimator<ScalarType>::estimateBPM3(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/100.0;
        
        PerTimeSliceStatistics<ScalarType> timeSliceStatistics(transformResult, timeSliceLength);
        timeSliceStatistics.calculateSum(/*0.0, transformResult->getOriginalDuration()*/);
        
        assert(timeSliceStatistics.getSumVector() != NULL);
        
        int maxDuration = timeSliceStatistics.getSumVector()->size();
        Eigen::VectorXd sumVec = *timeSliceStatistics.getSumVector();
        
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
        int maxCorrShift=std::min(int(6.0/timeSliceLength), maxDuration-1);
        Eigen::VectorXf derivSum(maxDuration-1);
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
        
        std::vector<int> beatDiffs;
        
        DEBUG_OUT("calculating bpm for slices of " << 2*maxCorrShift*timeSliceLength << " seconds, every " << maxCorrShift*timeSliceLength << " seconds.", 10)
        for (int i=0; i<derivSum.size() - 2*maxCorrShift; i+=maxCorrShift)
        {
            DEBUG_OUT("position: " << i*timeSliceLength, 12);
            DEBUG_OUT("calculating auto correlation of derivation vector...", 15);
            Eigen::VectorXf autoCorr(maxCorrShift);
            for (int shift=0; shift<maxCorrShift; shift++)
            {
                double corr=0.0;
                for (int relPos=0; relPos<maxCorrShift; relPos++)
                {
                    int shiftPos = i + relPos + shift;
                    corr += derivSum[i + relPos] * ((shiftPos < derivSum.size()) ? derivSum[shiftPos] : 0.0);
                }
                autoCorr[shift] = corr;
            }
            
            #if DEBUG_LEVEL>=25
                std::ofstream outstr4("autoCorr.dat");
                for (int j=1; j<autoCorr.size(); j++)
                {
                    outstr4 << autoCorr[j];
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
                DEBUG_OUT("mean: " << mean, 25);
                
                DEBUG_OUT("calculate variance...", 20);
                double variance=0.0;
                for (int i=0; i<autoCorr.size(); i++)
                {
                    double val = autoCorr[i] - mean;
                    variance += val*val;
                }
                variance /= autoCorr.size();
                double standardDerivation = sqrt(variance);
                DEBUG_OUT("variance: " << variance, 25);
                DEBUG_OUT("standard derivation: " << standardDerivation, 25);
                
                DEBUG_OUT("build barrier breaker list...", 20);
                double barrierVal = mean + standardDerivation;
                barrierVal = (barrierVal > 0.8*maxVal) ? 0.8*maxVal : barrierVal;
                DEBUG_OUT("barrier value: " << barrierVal, 25);
                int lastAddedPosition = -10000;
                int risePos=-1;
                for (int j=1; j<autoCorr.size()-1; j++)
                {
                    if ((j-lastAddedPosition) > 0.2/timeSliceLength)
                    {
                        if ((risePos < 0) && (autoCorr[j] > barrierVal) && (autoCorr[j] - autoCorr[j-1] > 0))   //rise
                        {
                            risePos = j;
                        }
                        else if ((risePos >= 0) && (autoCorr[j] < barrierVal) && (autoCorr[j+1] - autoCorr[j] < 0))   //fall
                        {
                            maxCorrPos.push_back(lastAddedPosition = risePos+(float(j-risePos)/2.0));
                            DEBUG_OUT("barrier breaker: " << lastAddedPosition, 25);
                            risePos=-1;
                        }
                    }
                }
            }
            
            DEBUG_OUT("calculating beat lengths...", 15);
            std::vector<float> diffPosVector;
            {
                double bpmMean = 0.0;
                float oldVal = *maxCorrPos.begin();
                for (std::vector<float>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
                {
                    float val = *it - oldVal;
                    diffPosVector.push_back(val);
                    beatDiffs.push_back(val);
                    DEBUG_OUT("beat length bpm: " << 60.0/(double(val)*timeSliceLength), 30);
                    DEBUG_OUT("val: " << double(val), 30);
                    bpmMean += val;
                    
                    oldVal = *it;
                }
                this->bpmMean = 60.0/(bpmMean / diffPosVector.size()*timeSliceLength);
                //sorting vector to get the median
                std::sort(diffPosVector.begin(), diffPosVector.end());
            }
            DEBUG_OUT("mean: " << colors::ConsoleColors::green() << this->bpmMean << colors::ConsoleColors::defaultText(), 0);
            DEBUG_OUT("median: " << colors::ConsoleColors::green() << 60.0/(double(diffPosVector[diffPosVector.size()/2])*timeSliceLength) << colors::ConsoleColors::defaultText(), 0);
            
            /*if (i*timeSliceLength >= 11)
                break;  //TODO: REMOVE!
            */
        }
        
        double mean=0.0;
        for (std::vector<int>::iterator it = beatDiffs.begin(); it != beatDiffs.end(); it++)
        {
            mean += *it;
        }
        mean /= beatDiffs.size();
        std::cerr << "meeeeean: " << colors::ConsoleColors::green() << 60.0/(mean*timeSliceLength) << colors::ConsoleColors::defaultText() << std::endl;
        
        #if 0
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<float> diffPosVector;
        {
            double bpmMean = 0.0;
            float oldVal = *maxCorrPos.begin();
            for (std::vector<float>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                float val = *it - oldVal;
                diffPosVector.push_back(val);
                DEBUG_OUT("beat length bpm: " << 60.0/(double(val)*timeSliceLength), 30);
                DEBUG_OUT("val: " << double(val), 30);
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
        for (std::vector<float>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceLength) - bpmMean;
            this->bpmVariance += val*val;
        }
        this->bpmVariance /= diffPosVector.size();
        
        DEBUG_OUT("cancel out outliers...", 15);
        double stdDeriv = sqrt(this->bpmVariance);
        double newMean = 0.0;
        std::vector<int> newDiffPosVector;
        double newVariance = 0.0;
        for (std::vector<float>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceLength) - bpmMean;
            std::cerr << val << "; " << stdDeriv << std::endl;
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
        this->bpmMean = 60.0/(newMean / newDiffPosVector.size()*timeSliceLength);
        #endif
    }
    
    template <typename ScalarType>
    void BPMEstimator<ScalarType>::estimateBPM1(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/100.0;
        
        PerTimeSliceStatistics<ScalarType> timeSliceStatistics(transformResult, timeSliceLength);
        timeSliceStatistics.calculateSum(/*0.0, transformResult->getOriginalDuration()*/);
        
        assert(timeSliceStatistics.getSumVector() != NULL);
        
        int maxDuration = timeSliceStatistics.getSumVector()->size();
        Eigen::VectorXd sumVec = *timeSliceStatistics.getSumVector();
        
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
        int maxCorrShift=std::min(int(6.0/timeSliceLength), maxDuration-1);
        Eigen::VectorXf derivSum(maxDuration-1);
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
                if ((i-lastAddedPosition) > 0.2/timeSliceLength)
                {
                    if ((risePos < 0) && (autoCorr[i] > barrierVal) && (autoCorr[i] - autoCorr[i-1] > 0))   //rise
                    {
                        std::cerr << "rise " << std::endl;
                        risePos = i;
                    }
                    else if ((risePos >= 0) && (autoCorr[i] < barrierVal) && (autoCorr[i+1] - autoCorr[i] < 0))   //fall
                    {
                        std::cerr << "fall " << std::endl;
                        maxCorrPos.push_back(lastAddedPosition = risePos+(float(i-risePos)/2.0));
                        std::cerr << lastAddedPosition << std::endl;
                        risePos=-1;
                    }
                }
            }
        }
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<float> diffPosVector;
        {
            double bpmMean = 0.0;
            float oldVal = *maxCorrPos.begin();
            for (std::vector<float>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                float val = *it - oldVal;
                diffPosVector.push_back(val);
                DEBUG_OUT("beat length bpm: " << 60.0/(double(val)*timeSliceLength), 30);
                DEBUG_OUT("val: " << double(val), 30);
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
        for (std::vector<float>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceLength) - bpmMean;
            this->bpmVariance += val*val;
        }
        this->bpmVariance /= diffPosVector.size();
        
        DEBUG_OUT("cancel out outliers...", 15);
        double stdDeriv = sqrt(this->bpmVariance);
        double newMean = 0.0;
        std::vector<int> newDiffPosVector;
        double newVariance = 0.0;
        for (std::vector<float>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 60.0/(double(*it)*timeSliceLength) - bpmMean;
            std::cerr << val << "; " << stdDeriv << std::endl;
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
        this->bpmMean = 60.0/(newMean / newDiffPosVector.size()*timeSliceLength);
    }
    
    template <typename ScalarType>
    void BPMEstimator<ScalarType>::estimateBPM2(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/200.0;
        
        PerTimeSliceStatistics<ScalarType> timeSliceStatistics(transformResult, timeSliceLength);
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
    
    template class BPMEstimator<kiss_fft_scalar>;
}

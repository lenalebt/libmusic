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
        estimateBPM4(transformResult);
    }
    
    void BPMEstimator::estimateBPM4(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/100.0;
        
        //PerTimeSliceStatistics timeSliceStatistics(transformResult, timeSliceLength);
        //timeSliceStatistics.calculateSum(0.0, transformResult->getOriginalDuration());
        
        int maxDuration = transformResult->getOriginalDuration()*(1.0/timeSliceLength);
        DEBUG_OUT("maxDuration: " << maxDuration, 25);
        Eigen::VectorXf sumVec(maxDuration);
        for (int i=0; i < maxDuration; i++)
        {
            double sum=0.0;
            for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
            {
                //sum only higher octaves?
                for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
                {
                    sum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeSliceLength, octave, bin));
                }
            }
            sumVec[i] = sum;
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
                if ((i-lastAddedPosition) > 0.1/timeSliceLength)
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
        
        while (this->bpmMean > 250)
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
    
    void BPMEstimator::estimateBPM3(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/200.0;
        int maxDuration = transformResult->getOriginalDuration()*(1.0/timeSliceLength);
        DEBUG_OUT("maxDuration: " << maxDuration, 25);
        Eigen::VectorXf sumVec(maxDuration);
        for (int i=0; i < maxDuration; i++)
        {
            double sum=0.0;
            for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
            {
                //sum only higher octaves?
                for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
                {
                    sum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeSliceLength, octave, bin));
                }
            }
            sumVec[i] = sum;
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
        
        DEBUG_OUT("calculating correlation with peak signal...", 15);
        double peakLength=0.0;
        double peakVal=-std::numeric_limits<double>::max();
        int peakPos=-1;
        {
            Eigen::VectorXf peakCorr(maxCorrShift/3-1);
            for (int i=0; i<peakCorr.size(); i++)
            {
                if (i>35)
                {
                    double var1 = autoCorr[2*i]+autoCorr[2*i+1];
                    double var2 = autoCorr[3*i]+autoCorr[3*i+1];
                    peakCorr[i] = sqrt(fabs(var1*var2));
                }
                else
                    peakCorr[i]=0;
            }
            for (int i=1; i<peakCorr.size(); i++)
            {
                if (peakCorr[i] > peakVal)
                {
                    peakVal=peakCorr[i];
                    peakPos = i;
                }
            }
            
            #if DEBUG_LEVEL>=25
                std::ofstream outstr5("peakCorr.dat");
                for (int i=1; i<peakCorr.size(); i++)
                {
                    outstr5 << peakCorr[i];
                    outstr5 << std::endl;
                }
            #endif
            
            DEBUG_OUT("peak found at i=" << peakPos << ", which means " << peakPos * 1000*timeSliceLength << "ms.", 20);
            
            
            while (peakPos*5 > 1500)
            {
                peakPos /= 2;
            }
            while (peakPos*5 < 200)
            {
                peakPos *= 2;
            }
            
            DEBUG_OUT("new estimate: " << peakPos * 1000*timeSliceLength << "ms.", 20);
            peakLength = peakPos * timeSliceLength;
            
            //std::cerr << "found speed: " << 60.0/(peakPos*timeSliceLength) << std::endl;
            
            DEBUG_OUT("getting more precision...", 10);
            for (int i=peakPos-10; i<peakPos+10; i++)
            {
                //TODO:
            }
            
            //okay, now we have an estimation that is correct up to multiples of i or 1/i with i being a power of 2 with
            //exponents n <= 1.
            std::cerr << "possible speeds: ";
            for (int i=-2; i<3; i++)
            {
                std::cerr << 60.0/peakLength * std::pow(2.0, i) << "; ";
            }
            std::cerr << std::endl;
            
            double maxVal=0.0;
            double aVal=0.0;
            double oVal=0.0;
            int maxValPos=-1;
            for (int i=3; i>-2; i--)
            {
                if (std::pow(2.0, i)*peakPos > peakCorr.size())
                    continue;
                aVal = peakCorr[std::pow(2.0, i)*peakPos];
                if (aVal > maxVal)
                {
                    maxVal = aVal;
                    maxValPos = i;
                    break;
                }
                oVal = aVal;
            }
            std::cerr << "found speed: " << 60.0/(std::pow(2.0, maxValPos)*peakPos*timeSliceLength) << std::endl;
        }
        
        
        
        
        
        
        
        
        
        
        
        
        
        /*
        
        DEBUG_OUT("calculating auto correlation of auto correlation vector...", 15);
        Eigen::VectorXf autoCorr2(maxCorrShift);
        for (int shift=0; shift<maxCorrShift; shift++)
        {
            double corr=0.0;
            for (int i=0; i<autoCorr2.size(); i++)
            {
                int shiftPos = i + shift;
                corr += autoCorr[i] * ((shiftPos < autoCorr.size()) ? autoCorr[shiftPos] : 0.0);
            }
            autoCorr2[shift] = corr;
        }
        
        DEBUG_OUT("looking for maxima...", 15);
        std::vector<int> maxCorrPos;
        {
            DEBUG_OUT("calculate mean...", 20);
            double mean=0.0;
            for (int i=0; i<autoCorr2.size(); i++)
            {
                mean += autoCorr2[i];
            }
            mean /= autoCorr2.size();
            
            DEBUG_OUT("calculate variance...", 20);
            double variance=0.0;
            for (int i=0; i<autoCorr2.size(); i++)
            {
                double val = autoCorr2[i] - mean;
                variance += val*val;
            }
            variance /= autoCorr2.size();
            double standardDerivation = sqrt(variance);
            
            double barrierVal = mean + standardDerivation;
            for (int i=0; i<autoCorr2.size(); i++)
            {
                if (autoCorr2[i] > barrierVal)
                    maxCorrPos.push_back(i);
            }
        }
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<int> diffPosVector;
        {
            int oldVal = *maxCorrPos.begin();
            for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                diffPosVector.push_back(*it - oldVal);
                //std::cerr << 30.0/(double(*it-oldVal)*timeSliceLength) << std::endl;
                oldVal = *it;
            }
            //sorting vector to get the median
            std::sort(diffPosVector.begin(), diffPosVector.end());
        }
        
        this->bpmMean = 30.0/(double(maxCorrPos[maxCorrPos.size()-1])/maxCorrPos.size()*timeSliceLength);
        this->bpmMedian = 30.0/(double(diffPosVector[diffPosVector.size()/2])*timeSliceLength);
        
        this->bpmVariance = 0.0;
        for (std::vector<int>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 30.0/(double(*it)*timeSliceLength) - bpmMean;
            this->bpmVariance += val*val;
        }
        this->bpmVariance /= diffPosVector.size();
        * */
    }
    
    void BPMEstimator::estimateBPM1(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        double* maxVals = new double[transformResult->getBinsPerOctave() * transformResult->getOctaveCount()];
        for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
        {
            for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
            {
                maxVals[octave*transformResult->getBinsPerOctave() + bin] = transformResult->getBinMean(octave, bin);
            }
        }
        
        DEBUG_OUT("sum all bins per time slice...", 15);
        //taking 5ms slices, summing values
        double timeSliceLength = 1.0/200.0;
        int maxDuration = transformResult->getOriginalDuration()*(1.0/timeSliceLength);
        Eigen::VectorXf sumVec(maxDuration);
        int maxSumPos=-1;
        double maxSum=-100.0;
        for (int i=0; i < maxDuration; i++)
        {
            double sum=0.0;
            for (int octave=0; octave<transformResult->getOctaveCount(); octave++)
            {
                //sum only higher octaves?
                for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
                {
                    sum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeSliceLength, octave, bin));// * maxVals[octave*transformResult->getBinsPerOctave() + bin];
                }
            }
            sumVec[i] = sum;
            
            /*
            //find steepest position
            if (maxSum < (sum-sumVec[i !=0 ? i-1  : 0]))
                maxSumPos=i;
            */
        }
        
        DEBUG_OUT("calculating auto correlation of sum vector...", 15);
        int maxCorrShift=6000;
        Eigen::VectorXf autoCorr(maxCorrShift);
        for (int shift=0; shift<maxCorrShift; shift++)
        {
            double corr=0.0;
            for (int i=0; i<sumVec.rows(); i++)
            {
                int shiftPos = i + shift;
                corr += sumVec[i] * ((shiftPos < sumVec.rows()) ? sumVec[shiftPos] : 0.0);
                //corr += pow(sumVec[i] - ((shiftPos < sumVec.rows()) ? sumVec[shiftPos] : 0.0), 2.0);
            }
            autoCorr[shift] = corr;
            //std::cerr << corr << std::endl; 
        }
        
        DEBUG_OUT("building derivation of auto correlation...", 15);
        Eigen::VectorXf derivCorr(maxCorrShift-1);
        for (int shift=0; shift<maxCorrShift-1; shift++)
        {
            derivCorr[shift] = autoCorr[shift+1] - autoCorr[shift];
        }
        
        DEBUG_OUT("looking for maxima...", 15);
        std::vector<int> maxCorrPos;
        int oldVal=0;
        for (int shift=1; shift<maxCorrShift-1; shift++)
        {
            if ((sgn(derivCorr[shift]) == +1) && (sgn(derivCorr[shift-1]) == -1))
            {   //change of sign in derivative from positive to negative! found max.
                maxCorrPos.push_back(shift);
                oldVal=shift;
            }
        }
        
        DEBUG_OUT("calculating beat lengths...", 15);
        std::vector<int> diffPosVector;
        {
            int oldVal = *maxCorrPos.begin();
            for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                diffPosVector.push_back(*it - oldVal);
                //std::cerr << 30.0/(double(*it-oldVal)*timeSliceLength) << std::endl;
                oldVal = *it;
            }
            //sorting vector to get the median
            std::sort(diffPosVector.begin(), diffPosVector.end());
        }
        
        this->bpmMean = 30.0/(double(maxCorrPos[maxCorrPos.size()-1])/maxCorrPos.size()*timeSliceLength);
        this->bpmMedian = 30.0/(double(diffPosVector[diffPosVector.size()/2])*timeSliceLength);
        
        this->bpmVariance = 0.0;
        for (std::vector<int>::iterator it = diffPosVector.begin(); it != diffPosVector.end(); it++)
        {
            double val = 30.0/(double(*it)*timeSliceLength) - bpmMean;
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
        int maxDuration = transformResult->getOriginalDuration()*(1.0/timeSliceLength);
        DEBUG_OUT("maxDuration: " << maxDuration, 25);
        Eigen::VectorXf sumVec(maxDuration);
        for (int i=0; i < maxDuration; i++)
        {
            double sum=0.0;
            for (int octave=0; octave<6/*transformResult->getOctaveCount()*/; octave++)
            {
                //sum only higher octaves?
                for (int bin=0; bin<transformResult->getBinsPerOctave(); bin++)
                {
                    sum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeSliceLength, octave, bin));
                }
            }
            sumVec[i] = sum;
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
        
        int offsetStep = 20.0/timeSliceLength;
        for (int offset = 0; offset < maxDuration-offsetStep; offset += offsetStep)
        {
            
            DEBUG_OUT("offset: " << offset, 12);
            DEBUG_OUT("calculating auto correlation of derivation vector...", 15);
            Eigen::VectorXf autoCorr(maxCorrShift);
            for (int shift=0; shift<maxCorrShift; shift++)
            {
                double corr=0.0;
                for (int i=offset; i<offset+offsetStep; i++)
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
            
            DEBUG_OUT("calculating correlation with peak signal...", 15);
            double peakLength=0.0;
            double peakVal=-std::numeric_limits<double>::max();
            int peakPos=-1;
            {
                Eigen::VectorXf peakCorr(maxCorrShift/3-1);
                for (int i=0; i<peakCorr.size(); i++)
                {
                    if (i>35)
                        peakCorr[i] = autoCorr[2*i]+autoCorr[2*i+1] + autoCorr[3*i]+autoCorr[3*i+1];
                    else
                        peakCorr[i]=0;
                }
                for (int i=1; i<peakCorr.size(); i++)
                {
                    if (peakCorr[i] > peakVal)
                    {
                        peakVal=peakCorr[i];
                        peakPos = i;
                    }
                }
                
                #if DEBUG_LEVEL>=25
                    std::ofstream outstr5("peakCorr.dat");
                    for (int i=1; i<peakCorr.size(); i++)
                    {
                        outstr5 << peakCorr[i];
                        outstr5 << std::endl;
                    }
                #endif
                
                DEBUG_OUT("peak found at i=" << peakPos << ", which means " << peakPos * 1000*timeSliceLength << "ms.", 20);
                
                while (peakPos*5 > 1500)
                {
                    peakPos /= 2;
                }
                while (peakPos*5 < 200)
                {
                    peakPos *= 2;
                }
                DEBUG_OUT("new estimate: " << peakPos * 1000*timeSliceLength << "ms.", 20);
                peakLength = peakPos * timeSliceLength;
                
                
                
                
                //okay, now we have an estimation that is correct up to multiples of i or 1/i with i being a power of 2 with
                //exponents n <= 1.
                std::cerr << "possible speeds: ";
                for (int i=-2; i<3; i++)
                {
                    std::cerr << 60.0/peakLength * std::pow(2.0, i) << "; ";
                }
                std::cerr << std::endl;
                
                double maxVal=0.0;
                double aVal=0.0;
                double oVal=0.0;
                int maxValPos=-1;
                for (int i=2; i>-3; i--)
                {
                    aVal = peakCorr[std::pow(2.0, i)*peakPos];
                    if ((aVal > peakVal * 0.2) && (aVal > peakCorr[std::pow(2.0, i-1)*peakPos]*1.5))
                    {
                        maxVal = aVal;
                        maxValPos = i;
                        break;
                    }
                    oVal = aVal;
                }
                std::cerr << "found speed: " << 60.0/(std::pow(2.0, maxValPos)*peakPos*timeSliceLength) << std::endl;
            }
        }
    }
}

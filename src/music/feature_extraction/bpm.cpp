#include "bpm.hpp"
#include "debug.hpp"

namespace music
{
    template <typename T> int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
    
    void BPMEstimator::estimateBPM(ConstantQTransformResult* transformResult)
    {
        estimateBPM3(transformResult);
    }
    void BPMEstimator::estimateBPM3(ConstantQTransformResult* transformResult)
    {
        DEBUG_OUT("estimating tempo of song...", 10);
        
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
                    sum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeSliceLength, octave, bin));
                }
            }
            sumVec[i] = sum;
            
            /*
            //find steepest position
            if (maxSum < (sum-sumVec[i !=0 ? i-1  : 0]))
                maxSumPos=i;
            */
        }
        
        DEBUG_OUT("building derivation of sum vector...", 15);
        int maxCorrShift=6000;
        Eigen::VectorXf derivSum(maxCorrShift-1);
        for (int shift=0; shift<maxCorrShift-1; shift++)
        {
            derivSum[shift] = sumVec[shift+1] - sumVec[shift];
        }
        
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
            //std::cerr << corr << std::endl;
        }
        
        /*
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
                //corr += sumVec[i] * ((shiftPos < sumVec.rows()) ? sumVec[shiftPos] : 0.0);
                corr += pow(sumVec[i] - ((shiftPos < sumVec.rows()) ? sumVec[shiftPos] : 0.0), 2.0);
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
}

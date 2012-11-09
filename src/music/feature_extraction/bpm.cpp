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
        
        
        /*
        DEBUG_OUT("estimating tempo for whole song...", 10);
        DEBUG_OUT("calculating auto correlation of sum vector...", 15);
        Eigen::VectorXf autoCorr(6000);
        for (int shift=0; shift<6000; shift++)
        {
            double corr=0.0;
            
            for (int i=0; i<sumVec.rows(); i++)
            {
                int shiftPos = i + shift;
                corr += sumVec[i] * ((shiftPos >= sumVec.rows()) ? 0.0 : sumVec[shiftPos]);
            }
            autoCorr[shift] = corr;
            //std::cerr << corr << std::endl;
        }
        
        DEBUG_OUT("building derivation of cross correlation...", 15);
        Eigen::VectorXf derivCorr(5999);
        for (int shift=0; shift<5999; shift++)
        {
            derivCorr[shift] = autoCorr[shift+1] - autoCorr[shift];
            //std::cerr << derivCorr[shift] << std::endl;
        }
        
        DEBUG_OUT("looking for maxima...", 15);
        std::vector<int> maxCorrPos;
        for (int shift=1; shift<5998; shift++)
        {
            if ((sgn(derivCorr[shift]) == +1) && (sgn(derivCorr[shift-1]) == -1))
            {   //change of sign in derivative from positive to negative! found max.
                maxCorrPos.push_back(shift);
            }
        }
        
        std::vector<int> diffPosVector;
        DEBUG_OUT("calculating BPM mean and variance...", 15);
        double bpmMean = 30.0/(double(maxCorrPos[maxCorrPos.size()-1])/maxCorrPos.size()*0.005);
        double bpmMedian=0.0;
        double bpmVariance=0.0;
        DEBUG_OUT("BPM mean: " << bpmMean, 15);
        
        int oldVal = *maxCorrPos.begin();
        for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
        {
            double val = 30.0/(double(*it - oldVal)*0.005) - bpmMean;
            bpmVariance += val*val;
            diffPosVector.push_back(*it - oldVal);
            //std::cerr << *it - oldVal << std::endl;
            oldVal = *it;
            //DEBUG_OUT("max found at " << *it, 30);
        }
        
        std::sort(diffPosVector.begin(), diffPosVector.end());
        bpmMedian = 30.0/(diffPosVector[diffPosVector.size()/2]*0.005);
        DEBUG_OUT("BPM median: " << bpmMedian, 15);
        
        bpmVariance /= maxCorrPos.size();
        DEBUG_OUT("BPM variance: " << bpmVariance, 15);
        */
        
        /*
        DEBUG_OUT("estimating tempo for parts of the song...", 10);
        for (int pos=0; pos<transformResult->getOriginalDuration()*100-2000; pos+=1000)
        {   //shift 10 seconds...
            Eigen::VectorXf autoCorr(2000);
            for (int shift=0; shift<2000; shift++)
            {
                double corr=0.0;
                
                for (int i=pos; i<pos+2000; i++)
                {
                    int shiftPos = i + shift;
                    corr += sumVec[i] * ((shiftPos >= sumVec.rows()) ? 0.0 : sumVec[shiftPos]);
                }
                autoCorr[shift] = corr;
                //std::cerr << corr << std::endl;
            }
            
            Eigen::VectorXf derivCorr(1999);
            for (int shift=0; shift<1999; shift++)
            {
                derivCorr[shift] = autoCorr[shift+1] - autoCorr[shift];
                //std::cerr << derivCorr[shift] << std::endl;
            }
            
            std::vector<int> maxCorrPos;
            for (int shift=1; shift<1998; shift++)
            {
                if ((sgn(derivCorr[shift]) == +1) && (sgn(derivCorr[shift-1]) == -1))
                {   //change of sign in derivative from positive to negative! found max.
                    maxCorrPos.push_back(shift);
                }
            }
            double bpmMean = 30.0/(double(maxCorrPos[maxCorrPos.size()-1])/maxCorrPos.size()*0.005);
            double bpmVariance=0.0;
            DEBUG_OUT("BPM mean: " << bpmMean, 15);
            
            int oldVal = *maxCorrPos.begin();
            for (std::vector<int>::iterator it = maxCorrPos.begin()+1; it != maxCorrPos.end(); it++)
            {
                double val = 30.0/(double(*it - oldVal)*0.005) - bpmMean;
                bpmVariance += val*val;
                oldVal = *it;
            }
            bpmVariance /= maxCorrPos.size();
            DEBUG_OUT("BPM variance: " << bpmVariance, 15);
        }
        
        DEBUG_OUT("TODO: cancel out outliers, without them, estimation should be good.", 15);
        */
        
    }
}

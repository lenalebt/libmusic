#ifndef BPM_HPP
#define BPM_HPP

#include "constantq.hpp"

namespace music
{
    class BPMEstimator
    {
    private:
        double bpmMean;
        double bpmMedian;
        double bpmVariance;
    protected:
        
    public:
        void estimateBPM(ConstantQTransformResult* transformResult);
        
        double getBPMMean() const       {return bpmMean;}
        double getBPMMedian() const     {return bpmMedian;}
        double getBPMVariance() const   {return bpmVariance;}
    };
}

#endif

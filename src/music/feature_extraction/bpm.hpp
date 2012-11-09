#ifndef BPM_HPP
#define BPM_HPP

#include "constantq.hpp"

namespace music
{
    /**
     * @todo Doku schreiben
     * @todo Implementierung überdenken/überarbeiten
     * @ingroup feature_extraction
     */
    template <typename Scalartype=kiss_fft_scalar>
    class BPMEstimator
    {
    private:
        double bpmMean;
        double bpmMedian;
        double bpmVariance;
        
        void estimateBPM1(ConstantQTransformResult* transformResult);
        void estimateBPM2(ConstantQTransformResult* transformResult);
        void estimateBPM3(ConstantQTransformResult* transformResult);
        void estimateBPM4(ConstantQTransformResult* transformResult);
        void estimateBPM5(ConstantQTransformResult* transformResult);
    protected:
        
    public:
        void estimateBPM(ConstantQTransformResult* transformResult);
        
        double getBPMMean() const       {return bpmMean;}
        double getBPMMedian() const     {return bpmMedian;}
        double getBPMVariance() const   {return bpmVariance;}
    };
}

#endif

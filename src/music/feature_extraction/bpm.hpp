#ifndef BPM_HPP
#define BPM_HPP

#include "constantq.hpp"
#include "feature_extraction_helper.hpp"

namespace music
{
    /**
     * @todo Doku schreiben
     * @todo Implementierung überdenken/überarbeiten
     * @ingroup feature_extraction
     */
    template <typename ScalarType=kiss_fft_scalar>
    class BPMEstimator
    {
    private:
        double bpmMean;
        double bpmMedian;
        double bpmVariance;
        
    protected:
        
    public:
        bool estimateBPM(PerTimeSliceStatistics<ScalarType>* timeSliceStatistics);
        
        double getBPMMean() const       {return bpmMean;}
        double getBPMMedian() const     {return bpmMedian;}
        double getBPMVariance() const   {return bpmVariance;}
    };
}

#endif

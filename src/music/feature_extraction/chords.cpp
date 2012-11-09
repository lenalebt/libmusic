#include "chords.hpp"

namespace music
{
    ChordEstimator::ChordEstimator(ConstantQTransformResult* transformResult) :
        transformResult(transformResult),
        timeResolution(timeResolution)
    {
        
    }
    Chord* ChordEstimator::estimateChord(double fromTime, double toTime)
    {
        
    }
}

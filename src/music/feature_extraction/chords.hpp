#ifndef CHORDS_HPP
#define CHORDS_HPP

#include <string>

namespace music
{
    //Holds one chord, e.g. c or A or D7.
    //add some similarity measure (zwei dur- oder zwei mollakkorde vergleichen, ...)
    class Chord
    {
    private:
        bool note[12];  //if a note is within this chord, or not
    protected:
        
    public:
        std::string getChordAsString();
        bool hasNote(int note);
    };

    class ChordEstimator
    {
    private:
        ConstantQTransformResult* transformResult;
        double timeResolution;
    protected:
        
    public:
        ChordEstimator(ConstantQTransformResult* transformResult, double timeResolution = 0.005);
        Chord* estimateChord(double fromTime, double toTime);
    };
}
#endif  //CHORDS_HPP

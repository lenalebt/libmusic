#ifndef CHORDS_HPP
#define CHORDS_HPP

#include <string>
#include "constantq.hpp"
#include <ostream>

#include <Eigen/Dense>

namespace music
{
    //Holds one chord, e.g. c or A or D7.
    //add some similarity measure (zwei dur- oder zwei mollakkorde vergleichen, ...)
    class Chord
    {
    private:
        Eigen::VectorXd chord;  //if a note is within this chord, or not
        
    protected:
        std::string getNoteName(int i);
    public:
        std::string getChordAsString();
        bool hasNote(int note);
        
        friend std::ostream& operator<<(std::ostream& os, const Chord& chord);
        friend class ChordEstimator;
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
    
    std::ostream& operator<<(std::ostream& os, const Chord& chord);
}
#endif  //CHORDS_HPP

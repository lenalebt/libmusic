#ifndef CHORDS_HPP
#define CHORDS_HPP

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
    
protected:
    
public:
    
};

#endif  //CHORDS_HPP

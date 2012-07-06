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
    /**
     * @todo Doku schreiben
     * @todo Implementierung überdenken
     */
    class ChordHypothesis
    {
    private:
        std::vector<std::pair<double, int> > hypothesis;
        Eigen::VectorXf normalizedChroma;
        float chromaMean;
        float chromaVariance;
        
    protected:
        static std::string getNoteName(int i);
        void createHypothesis();
    public:
        ChordHypothesis(int binsPerOctave=12);
        
        std::string getMaxHypothesisAsString();
        
        friend std::ostream& operator<<(std::ostream& os, const ChordHypothesis& chord);
        friend class ChordEstimator;
    };

    class ChordEstimator
    {
    private:
        ConstantQTransformResult* transformResult;
        double timeResolution;
    protected:
        ChordHypothesis* estimateChord1(double fromTime, double toTime);
        ChordHypothesis* estimateChord2(double fromTime, double toTime);
        ChordHypothesis* estimateChord3(double fromTime, double toTime);
    public:
        ChordEstimator(ConstantQTransformResult* transformResult, double timeResolution = 0.005);
        ChordHypothesis* estimateChord(double fromTime, double toTime);
    };
    
    std::ostream& operator<<(std::ostream& os, const ChordHypothesis& chord);
}
#endif  //CHORDS_HPP

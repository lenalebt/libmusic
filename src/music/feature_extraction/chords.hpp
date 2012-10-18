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
     * @ingroup feature_extraction
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
    /**
     * @ingroup feature_extraction
     */
    class ChordEstimator
    {
    private:
        ConstantQTransformResult* transformResult;
        double timeResolution;
        
        void applyNonlinearFunction(Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector);
    protected:
        ChordHypothesis* estimateChord1(double fromTime, double toTime);
        ChordHypothesis* estimateChord2(double fromTime, double toTime);
        ChordHypothesis* estimateChord3(double fromTime, double toTime);
    public:
        ChordEstimator(ConstantQTransformResult* transformResult, double timeResolution = 0.005);
        ChordHypothesis* estimateChord(double fromTime, double toTime);
        bool estimateChords(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chords, double timeSliceLength = 0.01, double timeResolution = 0.005);
    };
    
    std::ostream& operator<<(std::ostream& os, const ChordHypothesis& chord);
    
    class ChordModel
    {
    private:
        
    protected:
        
    public:
        
    };
}
#endif  //CHORDS_HPP

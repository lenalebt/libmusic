#include "chords.hpp"

#include <Eigen/Dense>
#include <complex>

namespace music
{
    ChordEstimator::ChordEstimator(ConstantQTransformResult* transformResult, double timeResolution) :
        transformResult(transformResult),
        timeResolution(timeResolution)
    {
        
    }
    Chord* ChordEstimator::estimateChord(double fromTime, double toTime)
    {
        if (fromTime < 0.0)
            fromTime = 0.0;
        if (toTime > transformResult->getOriginalDuration())
            toTime = transformResult->getOriginalDuration();
        
        assert(fromTime >= 0.0);
        assert(toTime >= fromTime);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        Chord* chord = new Chord();
        
        /* TODO:
         * -rechne binsumme aus, sodass 12 bins bleiben (einer pro ton). nimm
         *  die entsprechende zeitauflösung. möglicherweise geht das schneller als
         *  über die zugriffsfunktionen, aber das kann man später noch verbessern.
         * -finde die größten 3 oder 4 werte, und rechne darauf erstmal aus ob
         *  das ein akkord ist, der sinn macht (dur/moll). wenn ja, versuche noch
         *  mit dem vierten ton septime/none oder so zu finden. es müssen nicht die
         *  lautesten töne die sein, die den durakkord bilden. evtl kann man die varianz/
         *  mittelwerte zur hilfe nehmen
         */
        
        Eigen::VectorXd chroma(binsPerOctave);
        for (int bin=0; bin < binsPerOctave; bin++)
        {
            chroma[bin] = 0.0;
        }
        int maxElement = (toTime - fromTime) / timeResolution + fromTime / timeResolution;
        for (int i = fromTime / timeResolution; i < maxElement; i++)
        {
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                double binSum = 0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    binSum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                }
                chroma[bin] += binSum;
            }
        }
        
        //now estimate from chroma vector. first normalize the values.
        {
            double maxVal = -std::numeric_limits<double>::max();
            //find largest value in magnitude
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                if (fabs(chroma[bin]) > maxVal)
                    maxVal = fabs(chroma[bin]);
            }
            double maxValReciprocal = 1.0 / maxVal;
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                //multiplication is faster than division
                chroma[bin] *= maxValReciprocal;
                assert(chroma[bin] <= 1.0);
            }
        }
        
        chord->chord = chroma;
        
        return chord;
    }
    
    std::ostream& operator<<(std::ostream& os, const Chord& chord)
    {
        os << chord.chord << std::endl;
        return os;
    }
}

#include "chords.hpp"

#include <Eigen/Dense>
#include <complex>
#include <algorithm>
#include <vector>
#include <iostream>

namespace music
{
    std::string Chord::getNoteName(int i)
    {
        switch (i)
        {
            case 0:     return std::string("F");
            case 1:     return std::string("F#");
            case 2:     return std::string("G");
            case 3:     return std::string("G#");
            case 4:     return std::string("A");
            case 5:     return std::string("A#");
            case 6:     return std::string("B");
            case 7:     return std::string("C");
            case 8:     return std::string("C#");
            case 9:     return std::string("D");
            case 10:    return std::string("D#");
            case 11:    return std::string("E");
        }
    }
    ChordEstimator::ChordEstimator(ConstantQTransformResult* transformResult, double timeResolution) :
        transformResult(transformResult),
        timeResolution(timeResolution)
    {
        
    }
    Chord* ChordEstimator::estimateChord(double fromTime, double toTime)
    {
        return estimateChord2(fromTime, toTime);
    }
    Chord* ChordEstimator::estimateChord1(double fromTime, double toTime)
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
                //double val = 0.0;
                for (int octave=0+1; octave<octaveCount-3; octave++)
                {
                    //val = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                    binSum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                }
                chroma[bin] += binSum;
            }
        }
        
        std::vector<std::pair<double, int> > noteList;
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
                noteList.push_back(std::pair<double, int>(chroma[bin], bin));
                assert(chroma[bin] <= 1.0);
            }
        }
        std::sort(noteList.begin(), noteList.end());
        chord->chord = chroma;
        
        bool* baseChord = new bool[binsPerOctave];
        for (int i=0; i<binsPerOctave; i++)
            baseChord[i] = false;
        baseChord[noteList[binsPerOctave-1].second] = true;
        baseChord[noteList[binsPerOctave-2].second] = true;
        baseChord[noteList[binsPerOctave-3].second] = true;
        if (noteList[binsPerOctave-4].first > (noteList[binsPerOctave-3].first/noteList[binsPerOctave-2].first)*noteList[binsPerOctave-3].first)
            baseChord[noteList[binsPerOctave-4].second] = true;
        
        for (int i=0; i<binsPerOctave; i++)
            std::cerr << baseChord[i] << " ";
        std::cerr << std::endl;
        
        for (int i=0; i<binsPerOctave; i++)
        {
            if (baseChord[i] && baseChord[(i+4)%binsPerOctave] && baseChord[(i+7)%binsPerOctave])
            {
                std::cerr << "found chord: major " << chord->getNoteName(i) << std::endl;
            }
            if (baseChord[i] && baseChord[(i+3)%binsPerOctave] && baseChord[(i+7)%binsPerOctave])
            {
                std::cerr << "found chord: minor " << chord->getNoteName(i) << std::endl;
            }
        }
        
        delete baseChord;
        return chord;
    }
    Chord* ChordEstimator::estimateChord2(double fromTime, double toTime)
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
                //double val = 0.0;
                for (int octave=0+1; octave<octaveCount-3; octave++)
                {
                    //val = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                    binSum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                }
                chroma[bin] += binSum;
            }
        }
        
        std::vector<std::pair<double, int> > noteList;
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
                noteList.push_back(std::pair<double, int>(chroma[bin], bin));
                assert(chroma[bin] <= 1.0);
            }
        }
        std::sort(noteList.begin(), noteList.end());
        chord->chord = chroma;
        
        bool* baseChord = new bool[binsPerOctave];
        for (int i=0; i<binsPerOctave; i++)
            baseChord[i] = false;
        baseChord[noteList[binsPerOctave-1].second] = true;
        baseChord[noteList[binsPerOctave-2].second] = true;
        baseChord[noteList[binsPerOctave-3].second] = true;
        if (noteList[binsPerOctave-4].first > (noteList[binsPerOctave-3].first/noteList[binsPerOctave-2].first)*noteList[binsPerOctave-3].first)
            baseChord[noteList[binsPerOctave-4].second] = true;
        
        for (int i=0; i<binsPerOctave; i++)
            std::cerr << baseChord[i] << " ";
        std::cerr << std::endl;
        
        for (int i=0; i<binsPerOctave; i++)
        {
            if (baseChord[i] && baseChord[(i+4)%binsPerOctave] && baseChord[(i+7)%binsPerOctave])
            {
                std::cerr << "found chord: major " << chord->getNoteName(i) << std::endl;
            }
            if (baseChord[i] && baseChord[(i+3)%binsPerOctave] && baseChord[(i+7)%binsPerOctave])
            {
                std::cerr << "found chord: minor " << chord->getNoteName(i) << std::endl;
            }
        }
        
        delete baseChord;
        return chord;
    }
    
    std::ostream& operator<<(std::ostream& os, const Chord& chord)
    {
        os << chord.chord;
        return os;
    }
}

#include "chords.hpp"

#include <Eigen/Dense>
#include <complex>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>

#include "console_colors.hpp"

namespace music
{
    ChordHypothesis::ChordHypothesis(int binsPerOctave) :
        hypothesis(2*binsPerOctave),
        normalizedChroma(binsPerOctave),
        chromaMean(0.0),
        chromaVariance(0.0)
        
    {
        
    }
    
    std::string ChordHypothesis::getMaxHypothesisAsString()
    {
        std::string chordname("");
        if (hypothesis[hypothesis.size()-1].second/(hypothesis.size()/2) == 0)
            chordname += "major ";
        else if (hypothesis[hypothesis.size()-1].second/(hypothesis.size()/2) == 1)
            chordname += "minor ";
        chordname += getNoteName(hypothesis[hypothesis.size()-1].second);
        return chordname;
    }
    std::string ChordHypothesis::getNoteName(int i)
    {
        switch (i%12)
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
        return std::string("n/a");
    }
    ChordEstimator::ChordEstimator(ConstantQTransformResult* transformResult, double timeResolution) :
        transformResult(transformResult),
        timeResolution(timeResolution)
    {
        
    }
    ChordHypothesis* ChordEstimator::estimateChord(double fromTime, double toTime)
    {
        return estimateChord2(fromTime, toTime);
    }
    ChordHypothesis* ChordEstimator::estimateChord3(double fromTime, double toTime)
    {
        if (fromTime < 0.0)
            fromTime = 0.0;
        if (toTime > transformResult->getOriginalDuration())
            toTime = transformResult->getOriginalDuration();
        
        assert(fromTime >= 0.0);
        assert(toTime >= fromTime);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        ChordHypothesis* chord = new ChordHypothesis();
        
        Eigen::VectorXd chroma(binsPerOctave);
        for (int bin=0; bin < binsPerOctave; bin++)
        {
            chroma[bin] = 0.0;
        }
        
        int maxElement = transformResult->getOriginalDuration() / timeResolution;
        std::vector<std::pair<double, int> > chordLikelihood(2*binsPerOctave);
        int oldChord=0;
        int oldChordStartTime;
        for (int i = 0; i < maxElement; i++)
        {
            chroma *= 1.0-1.0/(0.5/timeResolution);
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                double binSum = 0.0;
                //double val = 0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    //val = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                    binSum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                }
                chroma[bin] += binSum/(0.5/timeResolution);
            }
            
            for (int j=0; j<binsPerOctave; j++)
            {
                chordLikelihood[j]               = std::pair<double,int>((chroma[j] + chroma[(j+4)%binsPerOctave] + chroma[(j+7)%binsPerOctave])/3.0, j);
                chordLikelihood[j+binsPerOctave] = std::pair<double,int>((chroma[j] + chroma[(j+3)%binsPerOctave] + chroma[(j+7)%binsPerOctave])/3.0, j + binsPerOctave);
            }
            std::sort(chordLikelihood.begin(), chordLikelihood.end());
            
            /*
            if ((oldChord != chordLikelihood[2*binsPerOctave-1].second) && (i-oldChordStartTime  > 0.125/timeResolution))
            {
                std::cerr << std::setw(10) << oldChordStartTime*timeResolution << ": ";
                std::cerr << (oldChord/binsPerOctave == 0 ? "major " : "minor ") << chord->getNoteName(oldChord%12) << std::endl;
                oldChord = chordLikelihood[2*binsPerOctave-1].second;
            }
            */
            std::cerr << std::setw(10) << i*timeResolution << ": ";
            std::cerr << (chordLikelihood[2*binsPerOctave-1].second/binsPerOctave == 0 ? "major " : "minor ") << chord->getNoteName(chordLikelihood[2*binsPerOctave-1].second%12) << std::endl;
        }
        
        
        
        
        

        
        return chord;
    }
    ChordHypothesis* ChordEstimator::estimateChord2(double fromTime, double toTime)
    {
        if (fromTime < 0.0)
            fromTime = 0.0;
        if (toTime > transformResult->getOriginalDuration())
            toTime = transformResult->getOriginalDuration();
        
        assert(fromTime >= 0.0);
        assert(toTime >= fromTime);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        ChordHypothesis* chordHypothesis = new ChordHypothesis(binsPerOctave);
        
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
            chordHypothesis->normalizedChroma[bin] = 0.0;
        }
        int maxElement = (toTime - fromTime) / timeResolution + fromTime / timeResolution;
        for (int i = fromTime / timeResolution; i < maxElement; i++)
        {
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                double binSum = 0.0;
                //double val = 0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    //val = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                    binSum += std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                }
                chordHypothesis->normalizedChroma[bin] += binSum;
            }
        }
        
        std::vector<std::pair<double, int> > noteList;
        //now estimate from chroma vector. first normalize the values.
        {
            double maxVal = -std::numeric_limits<double>::max();
            //find largest value in magnitude
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                if (fabs(chordHypothesis->normalizedChroma[bin]) > maxVal)
                    maxVal = fabs(chordHypothesis->normalizedChroma[bin]);
            }
            if (maxVal != 0.0)
            {
                double maxValReciprocal = 1.0 / maxVal;
                for (int bin=0; bin < binsPerOctave; bin++)
                {
                    //multiplication is faster than division
                    chordHypothesis->normalizedChroma[bin] *= maxValReciprocal;
                    assert(chordHypothesis->normalizedChroma[bin] <= 1.0);
                }
            }
        }
        
        chordHypothesis->createHypothesis();
        
        return chordHypothesis;
    }
    ChordHypothesis* ChordEstimator::estimateChord1(double fromTime, double toTime)
    {
        if (fromTime < 0.0)
            fromTime = 0.0;
        if (toTime > transformResult->getOriginalDuration())
            toTime = transformResult->getOriginalDuration();
        
        assert(fromTime >= 0.0);
        assert(toTime >= fromTime);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        ChordHypothesis* chord = new ChordHypothesis();
        
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
                for (int octave=0; octave<octaveCount; octave++)
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
        //chord->chord = chroma;
        
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
    
    void ChordHypothesis::createHypothesis()
    {
        for (int i=0; i<normalizedChroma.size(); i++)
        {
            hypothesis[i]               = std::pair<double,int>((normalizedChroma[i] + normalizedChroma[(i+4)%normalizedChroma.size()] + normalizedChroma[(i+7)%normalizedChroma.size()]), i);
            hypothesis[i+normalizedChroma.size()] = std::pair<double,int>((normalizedChroma[i] + normalizedChroma[(i+3)%normalizedChroma.size()] + normalizedChroma[(i+7)%normalizedChroma.size()]), i + normalizedChroma.size());
        }
        std::sort(hypothesis.begin(), hypothesis.end());
    }
    
    std::ostream& operator<<(std::ostream& os, const ChordHypothesis& chordHypothesis)
    {
        os << "chord: ";
        for (int i=chordHypothesis.hypothesis.size()-1; i>=chordHypothesis.hypothesis.size()-4; i--)
        {
            os << colors::ConsoleColors::green();
            if (chordHypothesis.hypothesis[i].second/(chordHypothesis.hypothesis.size()/2) == 0)
                os << "major ";
            else if (chordHypothesis.hypothesis[i].second/(chordHypothesis.hypothesis.size()/2) == 1)
                os << "minor ";
            os << std::setw(2) << ChordHypothesis::getNoteName(chordHypothesis.hypothesis[i].second%12) << colors::ConsoleColors::defaultText() << ": " << std::setw(7) << chordHypothesis.hypothesis[i].first << " ";
        }
        return os;
    }
}

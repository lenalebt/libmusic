#include "chords.hpp"

#include <Eigen/Dense>
#include <complex>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>

#include "console_colors.hpp"
#include "debug.hpp"

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
        /*std::string chordname("");
        if (hypothesis[hypothesis.size()-1].second/(hypothesis.size()/2) == 0)
            chordname += "major ";
        else if (hypothesis[hypothesis.size()-1].second/(hypothesis.size()/2) == 1)
            chordname += "minor ";
        chordname += getNoteName(hypothesis[hypothesis.size()-1].second);*/
        std::string chordname = getNoteName(hypothesis[hypothesis.size()-1].second);
        if (hypothesis[hypothesis.size()-1].second/(hypothesis.size()/2) == 1)
            std::transform(chordname.begin(), chordname.end(), chordname.begin(), ::tolower);
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
    
    void ChordEstimator::applyNonlinearFunction(Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector)
    {
        //vector = vector.array().pow(0.8);    //1.0236795
        //vector = vector.array().pow(1.0);    //1.0253195
        //vector = vector.array().pow(1.1);    //1.0256689
        vector = vector.array().pow(1.2);    //1.0257333
        //vector = vector.array().pow(1.3);    //1.0255818
        //vector = vector.array().pow(1.5);    //1.0247591
        //vector = vector.array().pow(2.0);    //1.0211522
        //vector = vector.array().pow(2.5);    //1.0171494
        //vector = vector.array().pow(3.0);    //works a bit better, but not good
    }
    
    bool ChordEstimator::estimateChords(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chords, double timeSliceLength, double timeResolution)
    {
        assert(timeResolution > 0.0);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        //init chroma to all zeroes.
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> chroma(binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> actChroma(binsPerOctave);
        Eigen::Matrix<double, Eigen::Dynamic, 1> overallChroma(binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> actChordLikelihood(2*binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> overallChordLikelihood(2*binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> actCQTmean(binsPerOctave * octaveCount);
        chroma.setZero();
        overallChroma.setZero();
        
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chromaVector;
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chordLikelihoodVector;
        
        int maxElement = transformResult->getOriginalDuration() / timeSliceLength;
        double time;
        for (int i = 1; i < maxElement; i++)
        {
            time = i * timeSliceLength;
            
            //calculate unsmoothed chroma of active time slice
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                double binSum = 0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    actCQTmean[octave * binsPerOctave + bin] = std::abs(transformResult->getNoteValueNoInterpolation(i*timeResolution, octave, bin));
                }
            }
            
            //apply a nonlinear function.
            //this step tries to cancel out overtones (hoping they are not
            //as loud as the loudest parts of the signal) and find the
            //relevant parts for chord estimation.
            double maxValue = actCQTmean.maxCoeff();
            actCQTmean /= maxValue;
            applyNonlinearFunction(actCQTmean);
            actCQTmean *= maxValue;
            
            
            //calculate new chroma values
            chroma *= 1.0-1.0/(0.5/timeResolution);     //exponential smoothing of the chroma vector
            //DEBUG_VAR_OUT(chroma.transpose(), 0);
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                double binSum = 0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    binSum += actCQTmean[octave * binsPerOctave + bin];
                }
                chroma[bin] += binSum/(0.5/timeResolution);     //exponential smoothing, see above
            }
            chromaVector.push_back(chroma);
            
            //DEBUG_VAR_OUT(chroma.transpose(), 0);
            
            overallChroma += chroma.cast<double>();
            
            /*
            //calculate the "likelihood" for chords. used to find out the mode of the recording
            for (int j=0; j<binsPerOctave; j++)
            {
                actChordLikelihood[j]               = (chroma[j] + chroma[(j+4)%binsPerOctave] + chroma[(j+7)%binsPerOctave])/3.0;
                actChordLikelihood[j+binsPerOctave] = (chroma[j] + chroma[(j+3)%binsPerOctave] + chroma[(j+7)%binsPerOctave])/3.0;
            }
            chordLikelihoodVector.push_back(actChordLikelihood);
            overallChordLikelihood += actChordLikelihood;
            * */
        }
        overallChroma /= maxElement;
        
        //okay, now calculate the mode of the song.
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> modeLikelihood(binsPerOctave);
        for (int j=0; j<binsPerOctave; j++)
        {
            modeLikelihood[j] = overallChroma[ j ]
                              + overallChroma[(j + 2)%binsPerOctave]
                              + overallChroma[(j + 4)%binsPerOctave]
                              + overallChroma[(j + 5)%binsPerOctave]
                              + overallChroma[(j + 7)%binsPerOctave]
                              + overallChroma[(j + 9)%binsPerOctave]
                              + overallChroma[(j +11)%binsPerOctave]
                              ;
        }
        
        std::cerr << modeLikelihood << std::endl;
        return true;
    }
    
    /**
     * @todo the algorithm behind estimateChord3 is more reliable,
     *      but slower on chord changes. use it nevertheless?
     */
    ChordHypothesis* ChordEstimator::estimateChord(double fromTime, double toTime)
    {
        //return estimateChord3(fromTime, toTime);
        return estimateChord1(fromTime, toTime);
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
                    binSum += std::abs(transformResult->getNoteValueMean(i*timeResolution, octave, bin, timeResolution));
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
        for (unsigned int i=chordHypothesis.hypothesis.size()-1; i>=chordHypothesis.hypothesis.size()-4; i--)
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

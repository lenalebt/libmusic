#include "chroma.hpp"

#include <Eigen/Dense>
#include <complex>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "console_colors.hpp"
#include "debug.hpp"

namespace music
{
    ChromaEstimator::ChromaEstimator(ConstantQTransformResult* transformResult) :
        transformResult(transformResult)
    {
        assert(transformResult != NULL);
    }
    
    void ChromaEstimator::applyNonlinearFunction(Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector)
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
    
    std::string ChromaEstimator::getNoteName(int i)
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
    
    GaussianMixtureModel<kiss_fft_scalar>* ChromaModel::getModel()
    {
        return model;
    }
    
    bool ChromaEstimator::estimateChroma(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& chromaVectors, int& mode, double timeSliceLength, bool makeTransposeInvariant)
    {
        assert(timeSliceLength > 0.0);
        
        int binsPerOctave = transformResult->getBinsPerOctave();
        int octaveCount = transformResult->getOctaveCount();
        
        chromaVectors.clear();
        
        //init chroma to all zeroes.
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> chroma(binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> actChroma(binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> overallChroma(binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> actChordLikelihood(2*binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> overallChordLikelihood(2*binsPerOctave);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> actCQTmean(binsPerOctave * octaveCount);
        chroma.setZero();
        overallChroma.setZero();
        overallChordLikelihood.setZero();
        
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chordLikelihoodVector;
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > localKeyChordLikelihoodVector;
        
        //calculate chroma vectors and chord likelihood vectors
        int maxElement = transformResult->getOriginalDuration() / timeSliceLength;
        int numValues = 0;
        double time;
        for (int i = 1; i < maxElement; i++)
        {
            time = i * timeSliceLength;
            
            //calculate unsmoothed chroma of active time slice
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                for (int octave=0; octave<octaveCount; octave++)
                {
                    actCQTmean[octave * binsPerOctave + bin] = std::abs(transformResult->getNoteValueMean(time, octave, bin, timeSliceLength));
                }
            }
            
            //apply a nonlinear function.
            //this step tries to cancel out overtones (hoping they are not
            //as loud as the loudest parts of the signal) and find the
            //relevant parts for chord estimation.
            double maxValue = fabs(actCQTmean.maxCoeff());
            if (maxValue > 1e-14)
            {
                actCQTmean /= maxValue;
                applyNonlinearFunction(actCQTmean);
                actCQTmean *= maxValue;
            }
            else
                continue;
            
            //calculate new chroma values
            chroma *= 1.0-(timeSliceLength/0.125);     //exponential smoothing of the chroma vector, ca. 1/8s "half-life time"
            //DEBUG_VAR_OUT(chroma.transpose(), 0);
            for (int bin=0; bin < binsPerOctave; bin++)
            {
                double binSum = 0.0;
                for (int octave=0; octave<octaveCount; octave++)
                {
                    binSum += actCQTmean[octave * binsPerOctave + bin];
                }
                chroma[bin] += binSum*(timeSliceLength/0.125);     //exponential smoothing, see above
            }
            
            chroma.normalize();
            chromaVectors.push_back(chroma);   //use normalized chroma vectors, don't want to depend on the volume
            
            overallChroma += chroma;
            
            //calculate the "likelihood" for chords. used to find out the mode of the recording
            for (int j=0; j<binsPerOctave; j++)
            {
                actChordLikelihood[j]               = (chroma[j] + chroma[(j+4)%binsPerOctave] + chroma[(j+7)%binsPerOctave])/3.0;
                actChordLikelihood[j+binsPerOctave] = (chroma[j] + chroma[(j+3)%binsPerOctave] + chroma[(j+7)%binsPerOctave])/3.0;
            }
            chordLikelihoodVector.push_back(actChordLikelihood);
            
            int maxLikelihoodChord = -1;
            {
                double maxLikelihoodValue = -std::numeric_limits<double>::max();
                for (int k = 0; k < 2*binsPerOctave; k++)
                {
                    if (actChordLikelihood[k] > maxLikelihoodValue)
                    {
                        maxLikelihoodChord = k;
                        maxLikelihoodValue = actChordLikelihood[k];
                    }
                }
                overallChordLikelihood[maxLikelihoodChord] += 1.0;
            }
            
            numValues++;
        }
        
        
        overallChroma /= numValues;
        
        /*
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
        DEBUG_VAR_OUT(modeLikelihood, 0);
        int maxLikelihoodMode = -1;
        {
            double maxLikelihoodValue = -std::numeric_limits<double>::max();
            for (int i = 0; i < binsPerOctave; i++)
            {
                if (modeLikelihood[i] > maxLikelihoodValue)
                {
                    maxLikelihoodMode = i;
                    maxLikelihoodValue = modeLikelihood[i];
                }
            }
        }
        DEBUG_OUT("maxlikelihood temporary mode from overallChroma: " << getNoteName(maxLikelihoodMode)
            << "/" << getNoteName(maxLikelihoodMode+9) << "m.", 20);*/
        
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> chordModeLikelihood(3*binsPerOctave);
        //major
        for (int j=0; j<binsPerOctave; j++)
        {   //lift tonic and dominant. they need to be there, so they are more important.
            chordModeLikelihood[j] = overallChordLikelihood[ j ]               * 3.0    //tonic
                              + overallChordLikelihood[(j + 5)%binsPerOctave]
                              + overallChordLikelihood[(j + 7)%binsPerOctave]  * 1.5    //dominant
                              + overallChordLikelihood[(j +14)%binsPerOctave+12]
                              + overallChordLikelihood[(j +16)%binsPerOctave+12]
                              + overallChordLikelihood[(j +21)%binsPerOctave+12]
                              ;
        }
        //minor (natural)
        for (int j=binsPerOctave; j<2*binsPerOctave; j++)
        {   //lift tonika and dominant. they need to be there, so they are more important.
            chordModeLikelihood[j] = overallChordLikelihood[ j%binsPerOctave + 12 ]         * 3.0    //tonic
                              + overallChordLikelihood[(j + 3)%binsPerOctave]
                              + overallChordLikelihood[(j + 8)%binsPerOctave]
                              + overallChordLikelihood[(j +10)%binsPerOctave]
                              + overallChordLikelihood[(j +17)%binsPerOctave+12]
                              + overallChordLikelihood[(j +19)%binsPerOctave+12] * 1.5     //dominant (minor)
                              ;
        }
        //minor (harmonic)
        for (int j=2*binsPerOctave; j<3*binsPerOctave; j++)
        {   //lift tonika and dominant. they need to be there, so they are more important.
            chordModeLikelihood[j] = overallChordLikelihood[ j%binsPerOctave + 12 ]      * 3.0    //tonic
                              + overallChordLikelihood[(j-12 + 3)%binsPerOctave]
                              + overallChordLikelihood[(j-12 + 8)%binsPerOctave]
                              + overallChordLikelihood[(j-12 +10)%binsPerOctave]
                              + overallChordLikelihood[(j-12 +17)%binsPerOctave+12]
                              + overallChordLikelihood[(j-12 + 7)%binsPerOctave] * 1.5     //dominant (major)
                              ;
        }
        //DEBUG_VAR_OUT(chordModeLikelihood, 0);
        int maxChordLikelihoodMode = -1;
        {
            double maxChordLikelihoodValue = -std::numeric_limits<double>::max();
            for (int i = 0; i < 3*binsPerOctave; i++)
            {
                if (chordModeLikelihood[i] > maxChordLikelihoodValue)
                {
                    maxChordLikelihoodMode = i;
                    maxChordLikelihoodValue = chordModeLikelihood[i];
                }
            }
        }
        DEBUG_OUT("maxlikelihood temporary mode from overallChordsLikelihood: " << getNoteName(maxChordLikelihoodMode)
        << ((maxChordLikelihoodMode>11) ? "m" : ""), 20);
        mode = maxChordLikelihoodMode;
        
        //now, we have some part of the mode: we know which notes are used in a recording.
        //this can lead to at least 2 parallel modes (major and minor).
        //the next step is used to differentiate between these two modes.
        
        if (makeTransposeInvariant)
        {
            DEBUG_OUT("making chroma transpose invariant...", 25);
            int shift = mode%binsPerOctave;
            Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmpVec(binsPerOctave);
            for (std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >::iterator it = chromaVectors.begin(); it != chromaVectors.end(); ++it)
            {
                for (int i = 0; i < binsPerOctave; i++)
                {
                    tmpVec[i] = (*it)[(i+shift)%binsPerOctave];
                }
                *it = tmpVec;
            }
        }
        
        return true;
    }
    
    bool ChromaModel::calculateChromaVectors(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& chromaVectors, double timeSliceLength, bool makeTransposeInvariant)
    {
        assert(timeSliceLength > 0.0);
        
        ChromaEstimator cEst(transformResult);
        bool retVal = cEst.estimateChroma(chromaVectors, mode, timeSliceLength, makeTransposeInvariant);
        assert(mode != -1);
        
        return retVal;
    }

    bool ChromaModel::calculateModel(unsigned int modelSize, double timeSliceLength, bool makeTransposeInvariant, ProgressCallbackCaller* callback)
    {
        if (model)
        {
            delete model;
            model = NULL;
        }
        model = new GaussianMixtureModelFullCov<kiss_fft_scalar>();
        
        if (callback)
            callback->progress(0.0, "initialized");
        
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > chroma;
        if (!this->calculateChromaVectors(chroma, timeSliceLength, makeTransposeInvariant))
            return false;
        
        DEBUG_VAR_OUT(chroma.size(), 0);
        
        #if DEBUG_LEVEL > 30
            std::ofstream outstr("chroma.dat");
            for (unsigned int i=0; i<chroma.size(); i++)
                outstr << chroma[i].transpose() << std::endl;
            outstr << std::endl;
        #endif
        
        if (chroma.size() < modelSize)
            return false;
        
        //then train the model (best-of-three).
        if (callback)
            callback->progress(0.5,  "calculating model 1");
        model->trainGMM(chroma, modelSize, 1e-8, 1e-10);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel1 = model->clone();
        if (callback)
            callback->progress(0.65, "calculating model 2");
        model->trainGMM(chroma, modelSize, 1e-8, 1e-10);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel2 = model->clone();
        if (callback)
            callback->progress(0.8,  "calculating model 3");
        model->trainGMM(chroma, modelSize, 1e-8, 1e-10);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel3 = model->clone();
        
        delete model;
        model = NULL;
        
        if (callback)
            callback->progress(0.95, "choose best model");
        
        //choose best-of-three
        if (tmpModel1->getModelLogLikelihood() > tmpModel2->getModelLogLikelihood())
        {
            if (tmpModel1->getModelLogLikelihood() > tmpModel3->getModelLogLikelihood())
            {
                model = tmpModel1;
                delete tmpModel2;
                delete tmpModel3;
            }
            else
            {
                model = tmpModel3;
                delete tmpModel1;
                delete tmpModel2;
            }
        }
        else
        {
            if (tmpModel2->getModelLogLikelihood() > tmpModel3->getModelLogLikelihood())
            {
                model = tmpModel2;
                delete tmpModel1;
                delete tmpModel3;
            }
            else
            {
                model = tmpModel3;
                delete tmpModel1;
                delete tmpModel2;
            }
        }
        
        if (callback)
            callback->progress(0.0, "finished");
        return true;
    }
    
    int ChromaModel::getMode()
    {
        return mode;
    }
    
    ChromaModel::ChromaModel(ConstantQTransformResult* transformResult) :
        transformResult(transformResult),
        model(NULL),
        mode(-1)
    {
        assert(transformResult != NULL);
    }
    ChromaModel::~ChromaModel()
    {
        if (model)
            delete model;
    }
}


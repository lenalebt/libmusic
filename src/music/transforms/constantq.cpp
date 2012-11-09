#include "constantq.hpp"

#include "debug.hpp"

//use for filtering
#include <musicaccess.hpp>

#include <assert.h>

#include <iostream>
#include <fstream>
#include <sstream>

#define MAX_FFT_LENGTH 2048

namespace music
{
    ConstantQTransform::ConstantQTransform() :
        octaveCount(8),
        fMin(40),
        kernelfMin(5000.0),
        fMax(11025),
        fs(22050),
        binsPerOctave(12),
        transpose(0.0),
        lowpassFilter(NULL),
        q(1.0),
        threshold(0.0005),
        fftHop(64),
        fftLen(128),
        atomNr(7),
        atomHop(0),
        nkMax(0),
        firstCenter(0),
        lastCenter(0),
        fKernel(NULL)
    {
        
    }
    
    ConstantQTransform* ConstantQTransform::createTransform(musicaccess::IIRFilter* lowpassFilter, int binsPerOctave, double fMin, double fMax, int fs, double q, double transpose, double threshold, double atomHopFactor)
    {
        assert(lowpassFilter != NULL);
        
        ConstantQTransform* cqt = NULL;
        cqt = new ConstantQTransform();
        assert(cqt != NULL);
        
        //recalculate fMax to be tied to a note. Apply transposing.
        {
            double k = ((12.0) * log2(fMax/440.0));
            do
            {
                fMax = 440.0 * pow(2.0, (std::floor(k) + transpose)/12.0);
                k--;
            }
            while (fMax > fs/2.0);
        }
        
        //set internal variables
        double fRatio = fMax/fMin;
        cqt->octaveCount = std::ceil(log2(fRatio));
        //recalculate the minimum frequency from the number of octaves involved such that we always calculate full octaves
        cqt->fMin = fMax / std::pow(2.0, cqt->octaveCount) * std::pow(2.0, 1.0/binsPerOctave);
        cqt->kernelfMin = (double(fMax)/2.0)*std::pow(2.0, 1.0/double(binsPerOctave));
        cqt->fMax = fMax;
        cqt->fs = fs;
        cqt->binsPerOctave = binsPerOctave;
        cqt->transpose = transpose;
        cqt->lowpassFilter = lowpassFilter;
        cqt->q = q;
        cqt->threshold = threshold;
        cqt->atomHopFactor = atomHopFactor;
        //this is what they use in the matlab implementation. Where went deltaOmega from eq.5 in the paper? shouldn't it only disappear in Nk?
        cqt->Q = q/(std::pow(2.0, 1.0/binsPerOctave) - 1);
        //calculate the length of the largest atom in samples
        cqt->nkMax = cqt->Q * double(fs) / double(cqt->kernelfMin) + 0.5;   //+0.5 for rounding
        //used in some formulas
        int ceil_nkMax_2 = std::ceil(double(cqt->nkMax)/2.0);
        
        //calculate the length of the shortest atom in samples
        int nkMin = cqt->Q * double(fs) / (double(cqt->kernelfMin) * std::pow(2.0, double(binsPerOctave-1)/double(binsPerOctave))) + 0.5;    //+0.5 for rounding
        
        cqt->atomHop = nkMin * atomHopFactor + 0.5;
        
        cqt->firstCenter = cqt->atomHop * std::ceil(double(ceil_nkMax_2)/cqt->atomHop);
        
        cqt->fftLen = pow(2.0, int(log2(cqt->firstCenter + ceil_nkMax_2))+1);    //the nextpow2 thing
        
        cqt->atomNr = std::floor(double(cqt->fftLen - ceil_nkMax_2 - cqt->firstCenter) / cqt->atomHop)+1;
        
        cqt->lastCenter = cqt->firstCenter + (cqt->atomNr-1) * cqt->atomHop;
        
        cqt->fftHop = (cqt->lastCenter + cqt->atomHop) - cqt->firstCenter;
        
        
        DEBUG_OUT("nkMax:" << cqt->nkMax, 10);
        DEBUG_OUT("ceil_nkMax_2:" << ceil_nkMax_2, 10);
        DEBUG_OUT("nkMin:" << nkMin, 10);
        DEBUG_OUT("atomHop:" << cqt->atomHop, 10);
        DEBUG_OUT("first_center:" << cqt->firstCenter, 10);
        DEBUG_OUT("FFTLen:" << cqt->fftLen, 10);
        DEBUG_OUT("atomNr:" << cqt->atomNr, 10);
        DEBUG_OUT("last_center:" << cqt->lastCenter, 10);
        DEBUG_OUT("FFTHop:" << cqt->fftHop, 10);
        
        
        //TODO: Calculate spectral kernels for one octave
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic>* tmpFKernel =
            new Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic>(cqt->fftLen, binsPerOctave * cqt->atomNr);     //fill into non-sparse matrix first, and then make sparse out of it (does not need that much memory)
        FFT fft;
        
        for (int bin = 1; bin <= binsPerOctave; bin++)
        {
            double fk = cqt->kernelfMin * std::pow(2.0, double(bin-1)/double(binsPerOctave));
            int nk = (cqt->Q * cqt->fs / fk ) + 0.5;   //+0.5 for rounding
            
            assert (nk <= cqt->nkMax);
            
            std::complex<kiss_fft_scalar>* tmpTemporalKernel = new std::complex<kiss_fft_scalar>[nk];
            
            std::complex<kiss_fft_scalar>* temporalKernel = new std::complex<kiss_fft_scalar>[cqt->fftLen];
            std::complex<kiss_fft_scalar>* spectralKernel = new std::complex<kiss_fft_scalar>[cqt->fftLen];
            
            //temporarily compute nonshifted temporal kernel (to speed up calculations later on)
            for (int i=0; i<nk; i++)
            {
                tmpTemporalKernel[i] = window<kiss_fft_scalar>(nk, i)/nk * exp( kiss_fft_scalar((2.0 * M_PI * fk ) / fs) * i * std::complex<kiss_fft_scalar>(0.0 ,1.0) );
            }
            
            //set temporal kernels. they are shifted versions of tmpTemporalKernel.
            for (int k=0; k<cqt->atomNr; k++)
            {
                int atomOffset = cqt->firstCenter-(nk/2+(nk&1));
                
                //initialize memory of temporal kernel to 0.0+0.0*i
                for (int i=0; i<cqt->fftLen; i++)
                {
                    temporalKernel[i]=0.0;
                }
                
                //fill temporal kernel
                for (int i=0; i<nk; i++)
                {
                    temporalKernel[(k*cqt->atomHop) + i + atomOffset] = tmpTemporalKernel[i];
                }
                
                //get spectral kernel from temporal kernel
                int fftlength=0;
                fft.docFFT((kiss_fft_cpx*)(temporalKernel), cqt->fftLen, (kiss_fft_cpx*)(spectralKernel), fftlength);
                assert(cqt->fftLen == fftlength);
                
                //copy to our dense matrix representation
                for (int i=0; i<cqt->fftLen; i++)
                {
                    if (abs(spectralKernel[i]) >= threshold)
                    {
                        std::complex<kiss_fft_scalar> val = spectralKernel[i];
                        val /= cqt->fftLen;
                        (*tmpFKernel)(i, (bin-1)*cqt->atomNr+k) = val;
                    }
                    else
                    {
                        (*tmpFKernel)(i, (bin-1)*cqt->atomNr+k) = std::complex<kiss_fft_scalar>(0.0, 0.0);
                    }
                }
            }
            delete[] tmpTemporalKernel;
            delete[] temporalKernel;
            
            delete[] spectralKernel;
        }
        
        DEBUG_OUT("normalizing magnitudes of kernel...", 15);
        //now normalize the magnitudes of the kernel
        int maxAPos=-1;
        int maxBPos=-1;
        {
            double maxA=-std::numeric_limits<double>::max();
            double maxB=-std::numeric_limits<double>::max();
            for (int i=0; i<tmpFKernel->rows(); i++)
            {
                if (std::abs((*tmpFKernel)(i,0)) > maxA)
                {
                    maxA=std::abs((*tmpFKernel)(i,0));
                    maxAPos=i;
                }
                if (std::abs((*tmpFKernel)(i,tmpFKernel->cols()-1)) > maxB)
                {
                    maxB=std::abs((*tmpFKernel)(i,tmpFKernel->cols()-1));
                    maxBPos=i;
                }
            }
            DEBUG_OUT("maxAPos+1=" << maxAPos+1 << ", value=" << (*tmpFKernel)(maxAPos,0), 30);
            DEBUG_OUT("maxBPos+1=" << maxBPos+1 << ", value=" << (*tmpFKernel)(maxBPos,tmpFKernel->cols()-1), 30);
        }
        
        Eigen::VectorXd weightVec(maxBPos - maxAPos - 2*int(1.0/q) + 1);
        for (int j=maxAPos + 1.0/q; j<=maxBPos - 1.0/q; j++)
        {
            double sum=0.0;
            for (int i=0; i<tmpFKernel->cols(); i++)
            {
                std::complex<kiss_fft_scalar> val = (*tmpFKernel)(j,i);
                sum += std::abs(val*std::conj(val));
            }
            DEBUG_OUT("weightVec[" << j-(maxAPos + 1.0/q) << "] = " << sum, 30);
            weightVec[j-(maxAPos + 1.0/q)] = sum;
        }
        
        DEBUG_OUT("calculating weight from weight vector...", 20);
        double weight = 0.0;
        {
            double mean=0.0;
            for (int i=0; i<weightVec.size(); i++)
            {
                mean += fabs(weightVec[i]);
            }
            mean /= weightVec.size();
            weight = sqrt(double(cqt->fftHop) / double(cqt->fftLen) * 1.0/mean);
        }
        DEBUG_OUT("weight = " << weight, 20);
        
        //copy the data from our tmpFKernel to our sparse fKernel.
        //also complex conjugate it.
        cqt->fKernel = new Eigen::SparseMatrix<std::complex<kiss_fft_scalar> >(binsPerOctave * cqt->atomNr, cqt->fftLen);
        for (int i=0; i<binsPerOctave * cqt->atomNr; i++)
        {
            for (int j=0; j<cqt->fftLen; j++)
            {
                if ((*tmpFKernel)(j,i) != std::complex<kiss_fft_scalar>(0.0, 0.0))
                {
                    std::complex<kiss_fft_scalar> value = (*tmpFKernel)(j,i);
                    value *= weight;
                    cqt->fKernel->insert(i, j) = std::conj(value);
                }
            }
        }
        delete tmpFKernel;
        
        #if DEBUG_LEVEL > 30
        {
            DEBUG_OUT("writing fkernel.csv with transform kernel...", 30)
            std::ofstream kernelstr("fkernel.csv");
            for (int i=0; i<cqt->getFKernel()->rows(); i++)
            {
                for (int j=0; j<cqt->getFKernel()->cols(); j++)
                {
                    kernelstr << (cqt->getFKernel()->coeff(i,j)) << ";";
                }
                kernelstr << std::endl;
            }
        }
        #endif
        
        //should now be able to do some cqt.
        return cqt;
    }
    
    ConstantQTransformResult* ConstantQTransform::apply(float* buffer, int sampleCount)
    {
        assert(this->lowpassFilter != NULL);
        assert(this->fKernel != NULL);
        
        //how many zeros should be padded to the lowest octave?
        int zeroPadding = fftLen/2;
        int maxBlock = fftLen * (1<<(octaveCount-1));
        
        int sampleCountWithBlock = sampleCount + 2*maxBlock;
        int originalSampleCount = sampleCount;
        
        FFT fft;
        //temporary fft data
        std::complex<kiss_fft_scalar>* fftData = NULL;
        fftData = new std::complex<kiss_fft_scalar>[fftLen];
        assert(fftData != NULL);
        
        float* data = new float[sampleCountWithBlock];
        for (int i=0; i<sampleCountWithBlock; i++)
        {
            if (i < maxBlock)
                data[i] = 0.0;
            else if (i >= maxBlock + sampleCount)
                data[i] = 0.0;
            else
                data[i] = buffer[i-maxBlock];
        }
        sampleCount = sampleCountWithBlock;
        
        float* fftSourceData=NULL;
        //used to calculate the fft with zero padding
        float* fftSourceDataZeroPadMemory=NULL;
        fftSourceDataZeroPadMemory = new float[fftLen];
        assert(fftSourceDataZeroPadMemory != NULL);
        
        //get memory for results
        ConstantQTransformResult* transformResult = NULL;
        transformResult = new ConstantQTransformResult();
        assert(transformResult != NULL);
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >* octaveResult = NULL;
        
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic> resultMatrix;
        
        transformResult->octaveMatrix = new Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >*[octaveCount];
        transformResult->drop = new int[octaveCount];
        assert(transformResult->octaveMatrix != NULL);
        assert(transformResult->drop != NULL);
        
        transformResult->originalZeroPadding = zeroPadding;
        
        int emptyHops = firstCenter / atomHop;
        
        //apply cqt once per octave
        for (int octave=octaveCount-1; octave >= 0; octave--)
        {
            //int overlap = fftLen - fftHop;        //needed in the matlab implementation, not needed here
            int fftlength=0;
            
            transformResult->drop[octave] = (emptyHops<<(octave)) - emptyHops;
            DEBUG_OUT("drop[" << octave << "] = " << transformResult->drop[octave], 12);
            
            octaveResult = NULL;
            octaveResult = new Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >(binsPerOctave, sampleCountWithBlock / fftHop * atomNr);
            assert(octaveResult != NULL);
            
            int windowNumber=0;
            //shift our window a bit. window has overlap.
            for (int position=0; position < sampleCountWithBlock-fftHop; position+=fftHop)
            {
                if (position + fftLen < sampleCountWithBlock)
                    fftSourceData = data + position;
                else
                {
                    fftSourceData = fftSourceDataZeroPadMemory;
                    for (int i=position; i<position+fftLen; i++)
                    {
                        if (i<sampleCountWithBlock)
                            fftSourceData[i-position] = data[i];
                        else
                            fftSourceData[i-position] = 0.0;
                    }
                }
                
                //apply FFT to input data.
                fft.doFFT((kiss_fft_scalar*)(fftSourceData), fftLen, (kiss_fft_cpx*)(fftData), fftlength);
                assert(fftlength == fftLen/2+1);
                
                //adjust fftData, as it only holds half of the data. the rest has to be computed.
                //complex conjugate, etc...
                int midPoint = fftLen/2;
                for (int i=1; i<midPoint; i++)
                {
                    fftData[midPoint + i] = conj(fftData[midPoint - i]);
                }
                //up to here: calculated FFT of the input data, one frame.
                
                //map fft Data vector to an Eigen data type
                Eigen::Map<Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, 1> > fftDataMap(fftData, fftLen);
                
                //Calculate the transform: apply fKernel to fftData.
                resultMatrix.noalias() = *fKernel * fftDataMap;
                //we get a matrix with (octaveCount*atomNr) x (1) elements.
                
                //reorder the result matrix, save data
                int winNrAtomNr = windowNumber * atomNr;
                int binAtomNr;
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    binAtomNr = bin*atomNr;
                    for (int i = 0; i < atomNr; i++)
                    {
                        (*octaveResult)(bin, winNrAtomNr + i) = resultMatrix(binAtomNr + i, 0);
                    }
                }
                windowNumber++;
            }
            
            transformResult->octaveMatrix[octave] = octaveResult;
            musicaccess::DownsamplingIIRFilter lowpassfilter_downsampling(lowpassFilter);
            
            DEBUG_OUT("apply low-pass filter", 30);
            if (octave)
            {   //not the last octave...
                //copy data to new array for filtering, if necessary
                if (octave == octaveCount)
                {
                    data = NULL;
                    data = new float[sampleCount];
                    assert(data != NULL);
                    memcpy(data, buffer, sampleCount * sizeof(float));
                }
                
                float* newData = NULL;
                newData = new float[sampleCount/2+1];
                assert(newData != NULL);
                
                //this IIR filter does downsampling and filtering in one step (about 10% faster than doing it in 2 steps)
                lowpassfilter_downsampling.applyDownsampling(data, sampleCount, newData);
                delete[] data;
                data = newData;
                sampleCount /= 2;
                sampleCountWithBlock /= 2;
            }
            else
            {
                if (octaveCount>1)
                    delete[] data;
            }
        }
        
        transformResult->minBinMidiNote = (12*log2(this->fMin/440.0))+69+transpose;
        transformResult->originalSamplingFrequency = this->fs;
        transformResult->originalSampleCount = originalSampleCount;
        transformResult->sampleCount = originalSampleCount + 2*maxBlock;
        transformResult->originalDuration = double(transformResult->originalSampleCount)/this->fs;
        transformResult->duration = double(transformResult->sampleCount)/this->fs;
        transformResult->timeFactor = transformResult->duration / transformResult->originalDuration;
        transformResult->binsPerOctave = this->binsPerOctave;
        transformResult->octaveCount = this->octaveCount;
        transformResult->fftLen = fftLen;
        transformResult->atomNr = atomNr;
        
        int droppedSamples = ((emptyHops<<(octaveCount-1)) - emptyHops - 1) * atomHop + firstCenter;
        
        transformResult->timeBefore = double(maxBlock)/this->fs - double(droppedSamples)/this->fs;
        //transformResult->timeBefore = double(droppedSamples)/this->fs - transformResult->duration * double(transformResult->drop[octaveCount-1])/double(transformResult->octaveMatrix[octaveCount-1]->cols());
        
        transformResult->timeAfter = double(maxBlock)/this->fs;
        
        delete[] fftData;
        delete[] fftSourceDataZeroPadMemory;
        
        #if DEBUG_LEVEL > 100
            DEBUG_OUT("saving data to files...", 10);
            for (int k=0; k<octaveCount; k++)
            {
                std::stringstream ss;
                ss << "octave" << k << ".dat";
                std::ofstream outstr(ss.str().c_str());
                DEBUG_OUT("file " << ss.str(), 10);
                for (int i=0; i < transformResult->octaveMatrix[k]->rows(); i++)
                {
                    for (int j=0; j < transformResult->octaveMatrix[k]->cols(); j++)
                    {
                        outstr << abs((*transformResult->octaveMatrix[k])(i,j)) << " ";
                    }
                    outstr << std::endl;
                }
            }
        #endif
        
        return transformResult;
    }
    
    ConstantQTransform::~ConstantQTransform()
    {
        if (fKernel)
            delete fKernel;
    }
    
    std::complex<kiss_fft_scalar> ConstantQTransformResult::getNoteValueNoInterpolation(float time, int octave, int bin) const
    {
        if (time <= 0.0f)
            return std::complex<kiss_fft_scalar>(0.0f, 0.0f);
        
        if (octave >= octaveCount)
        {
            DEBUG_OUT("tried to acces octave " << octave << ", we only have " << octaveCount << " octaves!", 10);
        }
        
        //time *= timeFactor;
        time += timeBefore;
        
        int pos = octaveMatrix[octaveCount-1]->cols();
        pos >>= octaveCount - octave - 1;
        pos *= (time/duration);
        //pos *= (time/originalDuration);
        pos += drop[octave] + 1;
        
        if (pos >= octaveMatrix[octave]->cols())
        {
            DEBUG_OUT("too large pos: " << pos, 25);
            return std::complex<kiss_fft_scalar>(0.0f, 0.0f);
        } else if (pos < 0)
        {
            DEBUG_OUT("too small pos: " << pos, 25);
            return std::complex<kiss_fft_scalar>(0.0f, 0.0f);
        }
        
        return (*octaveMatrix[octave])(bin, pos);
    }
    kiss_fft_scalar ConstantQTransformResult::getNoteValueMean(float time, int octave, int bin, float preDuration) const
    {
        if (time <= 0.0f)
            return 0.0f;
        
        assert(preDuration > 0);
        
        if (octave >= octaveCount)
        {
            DEBUG_OUT("tried to acces octave " << octave << ", we only have " << octaveCount << " octaves!", 10);
        }
        
        //time *= timeFactor;
        time += timeBefore;
        double preTime = time - preDuration;
        
        int pos = octaveMatrix[octaveCount-1]->cols();
        pos >>= octaveCount - octave - 1;
        pos *= (time/duration);
        //pos *= (time/originalDuration);
        pos += drop[octave] + 1;
        
        int prePos;
        if (preTime <= 0.0)
        {
            prePos=0;
            prePos += drop[octave] + 1;
        }
        else
        {
            prePos = octaveMatrix[octaveCount-1]->cols();
            prePos >>= octaveCount - octave - 1;
            prePos *= (preTime/duration);
            prePos += drop[octave] + 1;
        }
        
        float mean=0.0f;
        
        assert(pos < octaveMatrix[octave]->cols());
        assert(prePos <= pos);
        
        for (int i=prePos; i<=pos; i++)
        {
            mean += std::abs((*octaveMatrix[octave])(bin, i));
        }
        
        if (pos != prePos)
            mean /= pos-prePos+1;
        
        return mean;
    }
    std::complex<kiss_fft_scalar> ConstantQTransformResult::getNoteValueLinearInterpolation(float time, int octave, int bin) const
    {
        /*if (time <= 0.0f)
            return std::complex<kiss_fft_scalar>(0.0f, 0.0f);
        
        if (octave >= octaveCount)
        {
            DEBUG_OUT("tried to acces octave " << octave << ", we only have " << octaveCount << " octaves!", 10);
        }
        
        //time *= timeFactor;
        time += timeBefore;
        
        int pos = octaveMatrix[octaveCount-1]->cols();
        pos >>= octaveCount - octave - 1;
        double dblPos = pos;
        pos *= (time/duration);
        dblPos *= (time/duration);
        //pos *= (time/originalDuration);
        pos += drop[octave] + 1;
        dblPos += drop[octave] + 1;
        
        double percent = dblPos - pos;
        
        if (pos + 1 >= octaveMatrix[octave]->cols())
        {
            return (*octaveMatrix[octave])(bin, pos);
        }
        else if (pos >= octaveMatrix[octave]->cols())
        {
            DEBUG_OUT("too large pos: " << pos, 25);
            return std::complex<kiss_fft_scalar>(0.0f, 0.0f);
        } else if (pos < 0)
        {
            DEBUG_OUT("too small pos: " << pos, 25);
            return std::complex<kiss_fft_scalar>(0.0f, 0.0f);
        }
        
        
        return (1.0-percent)*(*octaveMatrix[octave])(bin, pos) + percent*(*octaveMatrix[octave])(bin, pos+1);*/
    }
    
    ConstantQTransformResult::~ConstantQTransformResult()
    {
        for (int i=0; i<octaveCount; i++)
        {
            delete octaveMatrix[i];
        }
        delete[] octaveMatrix;
    }
    
    double ConstantQTransformResult::getBinMax(int octave, int bin) const
    {
        assert(octave < octaveCount);
        assert(bin < binsPerOctave);
        assert(bin >= 0);
        assert(octave >= 0);
        
        double maxVal = std::numeric_limits<double>::min();
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >* octaveBins = octaveMatrix[octave];
        for (int i=0; i<octaveBins->cols(); i++)
        {
            double val = std::abs((*octaveBins)(bin, i));
            if (maxVal < val)
                maxVal = val;
        }
        return maxVal;
    }
    
    double ConstantQTransformResult::getBinMin(int octave, int bin) const
    {
        assert(octave < octaveCount);
        assert(bin < binsPerOctave);
        assert(bin >= 0);
        assert(octave >= 0);
        
        double minVal = std::numeric_limits<double>::max();
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >* octaveBins = octaveMatrix[octave];
        for (int i=0; i<octaveBins->cols(); i++)
        {
            double val = std::abs((*octaveBins)(bin, i));
            if (minVal < val)
                minVal = val;
        }
        return minVal;
    }
    
    double ConstantQTransformResult::getBinMean(int octave, int bin) const
    {
        assert(octave < octaveCount);
        assert(bin < binsPerOctave);
        assert(bin >= 0);
        assert(octave >= 0);
        
        double mean = 0.0;
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >* octaveBins = octaveMatrix[octave];
        for (int i=0; i<octaveBins->cols(); i++)
        {
            mean += std::abs((*octaveBins)(bin, i));
        }
        mean /= octaveBins->cols();
        return mean;
    }
    
    const Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic >* ConstantQTransformResult::getOctaveMatrix(int octave) const
    {
        assert (octave < octaveCount);
        assert (octave >= 0);
        
        return this->octaveMatrix[octave];
    }
}

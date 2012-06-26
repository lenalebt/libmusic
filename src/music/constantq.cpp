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
        nkMax(0),
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
        
        int ceil_nkMax_2 = std::ceil(double(cqt->nkMax)/2.0);
        
        //calculate the length of the shortest atom in samples
        int nkMin = cqt->Q * double(fs) / (double(cqt->kernelfMin) * std::pow(2.0, double(binsPerOctave-1)/double(binsPerOctave))) + 0.5;    //+0.5 for rounding
        
        int atomHop = nkMin * atomHopFactor + 0.5;
        
        int first_center = atomHop * std::ceil(double(ceil_nkMax_2)/atomHop);
        
        cqt->fftLen = pow(2.0, int(log2(first_center + ceil_nkMax_2))+1);    //the nextpow2 thing
        
        cqt->atomNr = std::floor(double(cqt->fftLen - ceil_nkMax_2 - first_center) / atomHop)+1;
        
        int last_center = first_center + (cqt->atomNr-1) * atomHop;
        
        cqt->fftHop = (last_center + atomHop) - first_center;
        
        
        DEBUG_OUT("nkMax:" << cqt->nkMax, 10);
        DEBUG_OUT("ceil_nkMax_2:" << ceil_nkMax_2, 10);
        DEBUG_OUT("nkMin:" << nkMin, 10);
        DEBUG_OUT("atomHop:" << atomHop, 10);
        DEBUG_OUT("first_center:" << first_center, 10);
        DEBUG_OUT("FFTLen:" << cqt->fftLen, 10);
        DEBUG_OUT("atomNr:" << cqt->atomNr, 10);
        DEBUG_OUT("last_center:" << last_center, 10);
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
                int atomOffset = first_center-(nk/2+(nk&1));
                
                //initialize memory of temporal kernel to 0.0+0.0*i
                for (int i=0; i<cqt->fftLen; i++)
                {
                    temporalKernel[i]=0.0;
                }
                
                //fill temporal kernel
                for (int i=0; i<nk; i++)
                {
                    temporalKernel[(k*atomHop) + i + atomOffset] = tmpTemporalKernel[i];
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
                        (*tmpFKernel)(i, (bin-1)*cqt->atomNr+k) = spectralKernel[i];
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
        
        #if DEBUG_LEVEL > 10
            std::ofstream outstr("tmpfkernel.dat");
            outstr << "tmpFKernel" << std::endl << *tmpFKernel << std::endl;
            
            DEBUG_OUT("tmpfkernel(" << tmpFKernel->rows() << "/" << tmpFKernel->cols() << ")", 10);
        #endif
        
        //copy the data from our tmpFKernel to our sparse fKernel.
        cqt->fKernel = new Eigen::SparseMatrix<std::complex<float> >(binsPerOctave * cqt->atomNr, cqt->fftLen);
        for (int i=0; i<binsPerOctave * cqt->atomNr; i++)
        {
            for (int j=0; j<cqt->fftLen; j++)
            {
                if ((*tmpFKernel)(j,i) != std::complex<kiss_fft_scalar>(0.0, 0.0))
                {
                    std::complex<kiss_fft_scalar> value = (*tmpFKernel)(j,i);
                    value /= cqt->fftLen;
                    cqt->fKernel->insert(i, j) = std::conj(value);
                }
            }
        }
        delete tmpFKernel;
        
        //comment in to see the sparse conjugate fKernel on the console.
        //std::cerr << "fKernel:" << *cqt->fKernel << std::endl;
        
        //should now be able to do some cqt.
        
        //TODO: weights are missing. do we need them? I think we would need them if we applied icqt.
        
        return cqt;
    }
    
    ConstantQTransformResult* ConstantQTransform::apply(float* buffer, int sampleCount)
    {
        assert(this->lowpassFilter != NULL);
        assert(this->fKernel != NULL);
        
        //how many zeros should be padded to the lowest octave?
        int zeroPadding = fftLen/2;
        
        FFT fft;
        //temporary fft data
        std::complex<float>* fftData = NULL;
        fftData = new std::complex<float>[fftLen];
        assert(fftData != NULL);
        
        float* data = buffer;
        float* fftSourceData=NULL;
        //used to calculate the fft with zero padding
        float* fftSourceDataZeroPadMemory=NULL;
        fftSourceDataZeroPadMemory = new float[fftLen];
        assert(fftSourceDataZeroPadMemory != NULL);
        
        //get memory for results
        ConstantQTransformResult* transformResult = NULL;
        transformResult = new ConstantQTransformResult();
        assert(transformResult != NULL);
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveResult = NULL;
        
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic> resultMatrix;
        
        transformResult->octaveMatrix = new Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >*[octaveCount];
        
        transformResult->originalDuration = double(sampleCount)/this->fs;
        transformResult->originalZeroPadding = zeroPadding;
        
        //apply cqt once per octave
        for (int octave=octaveCount-1; octave >= 0; octave--)
        {
            int overlap = fftLen - fftHop;        //needed in the matlab implementation, not needed here
            int fftlength=0;
            
            //int lZeros = (zeroPadding << octave) - ((zeroPadding << octave)/fftHop)*fftHop; //zeros padded on the left (for this octave)
            int lZeros = zeroPadding; //zeros padded on the left (for this octave)
            int rZeros = fftLen - (lZeros + sampleCount)%fftHop; //zeros padded on the right (for this octave)
            
            octaveResult = NULL;
            octaveResult = new Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >(binsPerOctave, ((sampleCount+lZeros)/fftHop +1) * atomNr);
            assert(octaveResult != NULL);
            
            int windowNumber=0;
            //shift our window a bit. window has overlap.
            for (int position=-lZeros; position < sampleCount; position+=fftHop)
            {
                if (position<0)
                {   //zero-padding necessary, front
                    fftSourceData = fftSourceDataZeroPadMemory;
                    //Fill array, zero-pad it as necessary (->beginning).
                    for (int i=position; i<position+fftLen; i++)
                    {
                        if (i<0)
                            fftSourceData[i-position] = 0.0;
                        else
                            fftSourceData[i-position] = data[i];
                    }
                }
                else if (position > sampleCount - zeroPadding - fftLen)
                {   //zero-padding necessary, back
                    fftSourceData = fftSourceDataZeroPadMemory;
                    //Fill array, zero-pad it as necessary (->end).
                    for (int i=position; i<position+fftLen; i++)
                    {
                        if (i>=sampleCount)
                            fftSourceData[i-position] = 0.0;
                        else
                            fftSourceData[i-position] = data[i];
                    }
                }
                else
                {   //no zero padding needed: use old buffer array
                    fftSourceData = data + position;
                }
                
                //apply FFT to input data.
                fft.doFFT((kiss_fft_scalar*)(fftSourceData), fftLen, (kiss_fft_cpx*)(fftData), fftlength);
                assert(fftlength == fftLen/2+1);
                
                //adjust fftData, as it only holds half of the data. the rest has to be computed.
                //complex conjugate, etc...
                int midPoint = fftLen/2;
                for (int i=1; i<fftLen/2; i++)
                {
                    fftData[midPoint + i] = conj(fftData[midPoint - i]);
                }
                //up to here: calculated FFT of the input data, one frame.
                
                //map fft Data vector to an Eigen data type
                Eigen::Map<Eigen::Matrix<std::complex<float>, Eigen::Dynamic, 1> > fftDataMap(fftData, fftLen);
                
                //Calculate the transform: apply fKernel to fftData.
                resultMatrix = *fKernel * fftDataMap;
                //we get a matrix with (octaveCount*atomNr) x (1) elements.
                
                //reorder the result matrix, save data
                for (int bin=0; bin<binsPerOctave; bin++)
                {
                    for (int i = 0; i < atomNr; i++)
                    {
                        (*octaveResult)(bin, windowNumber*atomNr + i) = resultMatrix(bin*atomNr + i, 0);
                    }
                }
                windowNumber++;
            }
            
            transformResult->octaveMatrix[octave] = octaveResult;
            
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
                
                //TODO: idea: implement new filter variant which only calculates every second value.
                //should speed it up 2x, and has better memory usage (can directly allocate half the
                //memory size).
                
                lowpassFilter->apply(data, sampleCount);
                
                //change samplerate
                float* newData = NULL;
                newData = new float[sampleCount/2];
                assert(newData != NULL);
                for (int i=0; i<sampleCount/2; i++)
                {
                    newData[i] = data[2*i];
                }
                delete[] data;
                data = newData;
                sampleCount /= 2;
            }
            else
            {
                if (octaveCount>1)
                    delete[] data;
            }
        }
        
        transformResult->minBinMidiNote = (12*log2(this->fMin/440.0))+69+transpose;
        transformResult->originalSamplingFrequency = this->fs;
        transformResult->originalSampleCount = sampleCount;
        transformResult->binsPerOctave = this->binsPerOctave;
        transformResult->octaveCount = this->octaveCount;
        transformResult->fftLen = fftLen;
        transformResult->atomNr = atomNr;
        
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
    
    std::complex<float> ConstantQTransformResult::getNoteValueNoInterpolation(float time, int octave, int bin) const
    {
        if (time <= 0.0f)
            return std::complex<float>(0.0f, 0.0f);
        
        if (octave >= octaveCount)
        {
            DEBUG_OUT("tried to acces octave " << octave << ", we only have " << octaveCount << " octaves!", 10);
        }
        
        int pos = octaveMatrix[octave]->cols() - atomNr*1.5;
        //pos -= (1<<octave);
        pos *= (time/originalDuration);
        pos += atomNr/2;   //first block was zero block
        //pos -= atomNr;   //first block was zero block
        //pos -= atomNr;   //last block was zero block
        
        
        if (pos >= octaveMatrix[octave]->cols())
        {
            DEBUG_OUT("too large pos: " << pos, 25);
            return std::complex<float>(0.0f, 0.0f);
        } else if (pos < 0)
        {
            DEBUG_OUT("too small pos: " << pos, 25);
            return std::complex<float>(0.0f, 0.0f);
        }
        
        return (*octaveMatrix[octave])(bin, pos);
    }
    std::complex<float> ConstantQTransformResult::getNoteValueLinearInterpolation(float time, int octave, int bin) const
    {
        return std::complex<float>(0.0f, 0.0f);
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
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveBins = octaveMatrix[octave];
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
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveBins = octaveMatrix[octave];
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
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveBins = octaveMatrix[octave];
        for (int i=0; i<octaveBins->cols(); i++)
        {
            mean += std::abs((*octaveBins)(bin, i));
        }
        mean /= octaveBins->cols();
        return mean;
    }
}

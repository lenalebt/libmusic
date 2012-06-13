#include "constantq.hpp"

//use for filtering
#include <musicaccess.hpp>

#include <assert.h>

#include <iostream>

namespace music
{
    ConstantQTransform::ConstantQTransform() :
        octaveCount(8),
        fMin(40),
        kernelfMin(5000.0),
        fMax(11025),
        fs(22050),
        binsPerOctave(12),
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
    
    ConstantQTransform* ConstantQTransform::createTransform(musicaccess::IIRFilter* lowpassFilter, int binsPerOctave, int fMin, int fMax, int fs, double q, double threshold, double atomHopFactor)
    {
        assert(lowpassFilter != NULL);
        
        ConstantQTransform* cqt = NULL;
        cqt = new ConstantQTransform();
        assert(cqt != NULL);
        
        //set internal variables
        double fRatio = fMax/fMin;
        cqt->octaveCount = std::ceil(log2(fRatio));
        //recalculate the minimum frequency from the number of octaves involved such that we always calculate full octaves
        cqt->fMin = fMax / std::pow(2.0, cqt->octaveCount) * std::pow(2.0, 1.0/binsPerOctave);
        cqt->kernelfMin = (double(fMax)/2.0)*std::pow(2.0, 1.0/double(binsPerOctave));
        cqt->fMax = fMax;
        cqt->fs = fs;
        cqt->binsPerOctave = binsPerOctave;
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
        
        /*
        std::cerr << "nkMax:" << cqt->nkMax << std::endl;
        std::cerr << "ceil_nkMax_2:" << ceil_nkMax_2 << std::endl;
        std::cerr << "nkMin:" << nkMin << std::endl;
        std::cerr << "atomHop:" << atomHop << std::endl;
        std::cerr << "first_center:" << first_center << std::endl;
        std::cerr << "FFTLen:" << cqt->fftLen << std::endl;
        std::cerr << "atomNr:" << cqt->atomNr << std::endl;
        std::cerr << "last_center:" << last_center << std::endl;
        std::cerr << "FFTHop:" << cqt->fftHop << std::endl;
        */
        
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
                        (*tmpFKernel)(i, bin-1 + k*cqt->atomNr) = spectralKernel[i];
                    }
                    else
                    {
                        (*tmpFKernel)(i, bin-1 + k*cqt->atomNr) = 0.0;
                    }
                }
            }
            delete[] tmpTemporalKernel;
            delete[] temporalKernel;
            
            delete[] spectralKernel;
        }
        
        //copy the data from our tmpFKernel to our sparse fKernel.
        cqt->fKernel = new Eigen::SparseMatrix<std::complex<float> >(cqt->fftLen, binsPerOctave * cqt->atomNr);
        for (int i=0; i<binsPerOctave * cqt->atomNr; i++)
        {
            for (int j=0; j<cqt->fftLen; j++)
            {
                if ((*tmpFKernel)(j,i) != 0.0)
                {
                    std::complex<kiss_fft_scalar> value = (*tmpFKernel)(j,i);
                    value /= cqt->fftLen;
                    cqt->fKernel->insert(j, i) = std::conj(value);
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
        
        //how many zeros should be padded?
        int zeroPadding = fftLen / 2;
        
        /*
        //we need to use std::complex<float> type to calculate the constant q transform.
        //this hurts in terms of memory usage, as we will occupy four times the memory
        //plus a bit for zero padding, effectively meaning we use about 10MiB/minute of music
        //in addition to the memory that is occupied by the original data.
        std::complex<float>* floatBuffer = new std::complex<float>[sampleCount + 2*zeroPadding];
        
        //copy input array to complex float array with zero padding (about 32kB zero padding for 8 octaves should be okay)
        for (int i=0; i<sampleCount + 2*zeroPadding; i++)
        {
            if ((i<zeroPadding) || (i>sampleCount + zeroPadding))
                floatBuffer[i] = 0.0;
            else
                floatBuffer[i] = std::complex<float>(float(buffer[i+zeroPadding]) / 32768.0f, 0.0);
        }
        */
        
        FFT fft;
        //temporary fft data
        std::complex<float>* fftData = new std::complex<float>[fftLen];
        float* data = buffer;
        float* fftSourceData;
        bool deleteFFTSourceData = false;
        
        //apply cqt once per octave
        for (int octave=octaveCount; octave > 0; octave--)
        {
            int overlap = fftLen - fftHop;
            int fftlength=0;
            //shift our window a bit. window has overlap.
            for (int position=-zeroPadding; position < sampleCount + zeroPadding ;position+=overlap)
            {
                if (position<0)
                {
                    deleteFFTSourceData = true;
                    fftSourceData = new float[fftLen];
                    //TODO: Fill array, zero-pad it as necessary (->beginning).
                }
                else if (position > sampleCount)
                {
                    deleteFFTSourceData = true;
                    fftSourceData = new float[fftLen];
                    //TODO: Fill array, zero-pad it as necessary (->end).
                }
                else
                {   //no zero padding needed: use old buffer array
                    deleteFFTSourceData = false;
                    fftSourceData = data + position;
                }
                
                //apply FFT to input data.
                fft.doFFT((kiss_fft_scalar*)(fftSourceData), fftLen, (kiss_fft_cpx*)(fftData), fftlength);
                assert(fftlength == fftLen/2+1);
                
                if (deleteFFTSourceData)
                    delete[] fftSourceData;
                
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
                Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic> resultMatrix;
                resultMatrix = fftDataMap * *fKernel;
                
                //TODO:reorder the result matrix, save data
            }
            
            if (octave)
            {   //not the last octave...
                //copy data to new array for filtering, if necessary
                if (octave == octaveCount)
                {
                    data = new float[sampleCount];
                    memcpy(data, buffer, sampleCount * sizeof(float));
                }
                
                //TODO: idea: implement new filter variant which only calculates every second value.
                //should speed it up 2x, and has better memory usage (can directly allocate half the
                //memory size).
                
                lowpassFilter->apply(data, sampleCount);
                
                //change samplerate
                float* newData = new float[sampleCount/2];
                for (int i=0; i<sampleCount/2; i++)
                {
                    newData[i] = data[2*i];
                }
                delete[] data;
                data = newData;
            }
        }
        
        delete[] fftData;
        
        //TODO: Map buffer to a Eigen vector via Map<>, see
        // http://eigen.tuxfamily.org/dox/TutorialMapClass.html
        //Eigen::Map<Eigen::Matrix<std::complex<float>, Eigen::Dynamic, 1> > bufferMap(floatBuffer);
        
        //then run over samples and calculate cqt.
        
        return NULL;
    }
    
    std::complex<float> ConstantQTransformResult::getNoteValueNoInterpolation(uint64_t sampleNumber, int midiNoteNumber) const
    {
        return std::complex<float>(0.0f, 0.0f);
    }
    std::complex<float> ConstantQTransformResult::getNoteValueLinearInterpolation(uint64_t sampleNumber, int midiNoteNumber) const
    {
        return std::complex<float>(0.0f, 0.0f);
    }
}

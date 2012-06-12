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
        
        //std::cerr << "nkMax:" << cqt->nkMax << std::endl;
        
        //calculate the length of the shortest atom in samples
        int nkMin = cqt->Q * double(fs) / (double(cqt->kernelfMin) * std::pow(2.0, double(binsPerOctave-1)/double(binsPerOctave))) + 0.5;    //+0.5 for rounding
        
        //std::cerr << "nkMin:" << nkMin << std::endl;
        
        int atomHop = nkMin * atomHopFactor + 0.5;
        
        //std::cerr << "atomHop:" << atomHop << std::endl;
        
        int first_center = atomHop * std::ceil(std::ceil(cqt->nkMax/2.0)/atomHop);
        
        //std::cerr << "first_center:" << first_center << std::endl;
        
        int FFTLen = pow(2.0, int(log2(first_center + std::ceil(cqt->nkMax/2.0)))+1);    //the nextpow2 thing
        
        //std::cerr << "FFTLen:" << FFTLen << std::endl;
        
        //TODO: Calculate spectral kernels for one octave
        Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic> tmpFKernel(binsPerOctave, FFTLen);     //fill into non-sparse matrix first, and then make sparse out of it (does not need that much memory)
        FFT fft;
        
        for (int bin = 1; bin <= binsPerOctave; bin++)
        {
            double fk = cqt->kernelfMin * std::pow(2.0, double(bin-1)/double(binsPerOctave));
            int Nk = (cqt->Q * cqt->fs / fk ) + 0.5;   //+0.5 for rounding
            
            std::complex<kiss_fft_scalar>* temporalKernel = new std::complex<kiss_fft_scalar>[FFTLen];
            std::complex<kiss_fft_scalar>* spectralKernel = new std::complex<kiss_fft_scalar>[FFTLen];
            
            for (int i=0; i<FFTLen; i++)
            {
                temporalKernel[i]=0;
            }
            
            for (int i=0; i<Nk; i++)
            {
                temporalKernel[i+first_center-(Nk/2+(Nk&1))] = window<kiss_fft_scalar>(Nk, i)/Nk * exp( kiss_fft_scalar((2.0 * M_PI * fk ) / fs) * i * std::complex<kiss_fft_scalar>(0.0 ,1.0) );
                                                   //^^this rounds up if necessary.
                if (bin==1)
                    std::cerr << temporalKernel[i+first_center-(Nk/2+(Nk&1))] << " " << std::endl;
                    //std::cerr << window<float>(Nk, i)/Nk << " " << std::endl;
            }
            
            int fftlength=0;
            fft.docFFT((kiss_fft_cpx*)temporalKernel, FFTLen, (kiss_fft_cpx*)spectralKernel, fftlength);
            std::cerr << Nk << " " << fftlength << std::endl;
            
            //we have our spectral kernel now in spectralKernel. save it!
            
            for (int i=0; i<FFTLen; i++)
            {
                tmpFKernel(bin-1, i) = spectralKernel[i];
                if (bin==1)
                    std::cerr << " " << (abs(spectralKernel[i]) > threshold ? spectralKernel[i] : 0.0) << std::endl;
            }
            
            
            delete[] temporalKernel;
            delete[] spectralKernel;
        }
        
        cqt->fKernel = new Eigen::SparseMatrix<std::complex<kiss_fft_scalar> >(binsPerOctave, FFTLen);
        for (int i=0; i<binsPerOctave; i++)
        {
            for (int j=0; j<FFTLen; j++)
            {
                if (abs(tmpFKernel(i,j)) >= threshold)
                    cqt->fKernel->insert(i, j) = tmpFKernel(i,j);
            }
        }
        
        //should now be able to do some cqt.
        
        return cqt;
    }
    
    ConstantQTransformResult* ConstantQTransform::apply(uint16_t* buffer, int sampleCount)
    {
        assert(this->lowpassFilter != NULL);
        assert(this->fKernel != NULL);
        //TODO: Map buffer to a Eigen vector via Map<>, see
        // http://eigen.tuxfamily.org/dox/TutorialMapClass.html
        
        //padding with zeros in the beginning and in the end
        
        //then run over samples and calculate cqt.
        //in the end, we will get a matrix with roughly (binsPerOctave*octaveCount)x(sampleCount/timesliceLength) entries.
        //that matrix will have more entries for higher frequencies, and lesser for lower frequencies.
    }
}

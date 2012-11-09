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
            new Eigen::Matrix<std::complex<kiss_fft_scalar>, Eigen::Dynamic, Eigen::Dynamic>(binsPerOctave * cqt->atomNr, cqt->fftLen);     //fill into non-sparse matrix first, and then make sparse out of it (does not need that much memory)
        FFT fft;
        
        for (int bin = 1; bin <= binsPerOctave; bin++)
        {
            double fk = cqt->kernelfMin * std::pow(2.0, double(bin-1)/double(binsPerOctave));
            int nk = (cqt->Q * cqt->fs / fk ) + 0.5;   //+0.5 for rounding
            
            assert (nk <= cqt->nkMax);
            
            std::complex<kiss_fft_scalar>* tmpTemporalKernel = new std::complex<kiss_fft_scalar>[nk];
            
            std::complex<kiss_fft_scalar>* temporalKernel = new std::complex<kiss_fft_scalar>[cqt->fftLen * cqt->atomNr];
            std::complex<kiss_fft_scalar>* spectralKernel = new std::complex<kiss_fft_scalar>[cqt->fftLen * cqt->atomNr];
            
            //temporarily compute nonshifted temporal kernel (to speed up calculations later on)
            for (int i=0; i<nk; i++)
            {
                tmpTemporalKernel[i] = window<kiss_fft_scalar>(nk, i)/nk * exp( kiss_fft_scalar((2.0 * M_PI * fk ) / fs) * i * std::complex<kiss_fft_scalar>(0.0 ,1.0) );
                if (bin==1)
                    std::cerr << temporalKernel[i+first_center-(nk/2+(nk&1))] << " " << std::endl;
                    //std::cerr << window<float>(nk, i)/nk << " " << std::endl;
            }
            
            //initialize memory of temporal kernels to 0.0+0.0*i
            for (int i=0; i<cqt->fftLen * cqt->atomNr; i++)
            {
                temporalKernel[i]=std::complex<kiss_fft_scalar>(0.0, 0.0);
            }
            
            //set temporal kernels. they are shifted versions of tmpTemporalKernel.
            for (int k=0; k<cqt->atomNr; k++)
            {
                int atomOffset = first_center-(nk/2+(nk&1));
                for (int i=0; i<nk; i++)
                {
                    temporalKernel[(k*atomHop) + i + atomOffset] = tmpTemporalKernel[i];
                }
                int fftlength=0;
                fft.docFFT((kiss_fft_cpx*)(temporalKernel+k*atomHop), cqt->fftLen, (kiss_fft_cpx*)(spectralKernel+k*atomHop), fftlength);
                assert(cqt->fftLen == fftlength);
            }
            delete[] tmpTemporalKernel;
            delete[] temporalKernel;
            
            //we have our spectral kernels now in spectralKernel. save it!
            for (int k=0; k<cqt->atomNr; k++)
            {
                for (int i=0; i<cqt->fftLen; i++)
                {
                    (*tmpFKernel)(bin-1 + k*cqt->atomNr, i) = spectralKernel[i];
                }
            }
            
            delete[] spectralKernel;
        }
        
        //copy the data from our tmpFKernel to our sparse fKernel.
        cqt->fKernel = new Eigen::SparseMatrix<std::complex<kiss_fft_scalar> >(binsPerOctave * cqt->atomNr, cqt->fftLen);
        for (int i=0; i<binsPerOctave * cqt->atomNr; i++)
        {
            for (int j=0; j<cqt->fftLen; j++)
            {
                if (abs((*tmpFKernel)(i,j)) >= threshold)
                    cqt->fKernel->insert(i, j) = conj((*tmpFKernel)(i,j));
            }
        }
        delete tmpFKernel;
        
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
        
        return NULL;
    }
}

#include "fft.hpp"
#include "debug.hpp"

namespace music
{
    FFT::FFT()
    {
        for (int i=0; i<FFT_PREFACTOR_COUNT; i++)
        {
            int timeLength = 1 << (i+1);
            //init kissfft
            rCfg[i] = kiss_fftr_alloc(timeLength, 0, NULL, NULL);
            cCfg[i] = kiss_fft_alloc(timeLength, 0, NULL, NULL);
            
            //init inverse kissfft
            riCfg[i] = kiss_fftr_alloc(timeLength, 1, NULL, NULL);
            ciCfg[i] = kiss_fft_alloc(timeLength, 1, NULL, NULL);
            
            //DEBUG_OUT(rCfg << " " << rCfg[0] << " " << rCfg[1], 10);
        }
    }
    FFT::~FFT()
    {
        for (int i=0; i<FFT_PREFACTOR_COUNT; i++)
        {
            free(rCfg[i]);
            free(cCfg[i]);
            
            free(riCfg[i]);
            free(ciCfg[i]);
        }
    }
    
    void FFT::doFFT(const kiss_fft_scalar *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength)
    {
        freqLength = timeLength/2 +1;
        
        //choose the right cfg
        int i=0;
        for (i=0; i<FFT_PREFACTOR_COUNT; i++)
        {
            if (timeLength & 2)
                break;
            timeLength >>= 1; 
        }
        //start kissfft
        kiss_fftr(rCfg[i], timeData, freqData);
    }
    
    void FFT::docFFT(const kiss_fft_cpx *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength)
    {
        freqLength = timeLength;
        
        //choose the right cfg
        int i=0;
        for (i=0; i<FFT_PREFACTOR_COUNT; i++)
        {
            if (timeLength & 2)
                break;
            timeLength >>= 1; 
        }
        //start kissfft
        kiss_fft(cCfg[i], timeData, freqData);
    }
    
    void FFT::doiFFT(const kiss_fft_cpx *freqData, int freqLength, kiss_fft_scalar *timeData, int& timeLength)
    {
        //choose the right cfg
        int i=0;
        for (i=0; i<FFT_PREFACTOR_COUNT; i++)
        {
            if (timeLength & 2)
                break;
            timeLength >>= 1; 
        }
        //start inverse kissfft
        kiss_fftri(riCfg[i], freqData, timeData);
        timeLength = 2*(freqLength-1);
    }
    
    /*
    void FFT::doicFFT(const kiss_fft_cpx *freqData, int freqLength, kiss_fft_cpx *timeData, int& timeLength)
    {
        //choose the right cfg
        int i=0;
        for (i=0; i<FFT_PREFACTOR_COUNT; i++)
        {
            if (timeLength & 2)
                break;
            timeLength >>= 1; 
        }
        //start inverse kissfft
        kiss_ffti(ciCfg[i], freqData, timeData);
        timeLength = 2*(freqLength-1);
    }
    */
}

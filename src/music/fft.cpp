#include "fft.hpp"

namespace music
{
    void FFT::doFFT(const kiss_fft_scalar *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength)
    {
        //init kissfft
        kiss_fftr_cfg cfg = kiss_fftr_alloc(timeLength, 0, NULL, NULL);
        
        //start kissfft
        kiss_fftr(cfg, timeData, freqData);
        
        freqLength = timeLength/2 +1;
        
        free(cfg);
    }
    
    void FFT::docFFT(const kiss_fft_cpx *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength)
    {
        //init kissfft
        kiss_fft_cfg cfg = kiss_fft_alloc(timeLength, 0, NULL, NULL);
        
        //start kissfft
        kiss_fft(cfg, timeData, freqData);
        
        freqLength = timeLength;
        
        free(cfg);
    }
    
    void FFT::doiFFT(const kiss_fft_cpx *freqData, int freqLength, kiss_fft_scalar *timeData, int& timeLength)
    {
        //init kissfft
        kiss_fftr_cfg cfg = kiss_fftr_alloc(freqLength, 1, NULL, NULL);
        
        //start kissfft
        kiss_fftri(cfg, freqData, timeData);
        
        timeLength = 2*(freqLength-1);
        
        free(cfg);
    }
}

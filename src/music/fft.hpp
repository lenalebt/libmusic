#ifndef FFT_HPP
#define FFT_HPP

#include "kiss_fftr.h"
#include "kiss_fft.h"

namespace music
{
    #define FFT_PREFACTOR_COUNT 12
    class FFT
    {
    private:
        kiss_fftr_cfg rCfg[FFT_PREFACTOR_COUNT];
        kiss_fft_cfg  cCfg[FFT_PREFACTOR_COUNT];
        kiss_fftr_cfg riCfg[FFT_PREFACTOR_COUNT];
        kiss_fft_cfg  ciCfg[FFT_PREFACTOR_COUNT];
    public:
        FFT();
        ~FFT();
        
        /**
         * @brief Performs a FFT with real input values.
         * 
         * Uses kissfft.
         * 
         * @param timeData The data in the time domain. This is an array of floats.
         * @param timeLength The length of the time data array. Should be a power of 2.
         * @param freqData Returns the data in the frequency domain.
         * @param freqLength[out] returns the length of the frequency
         *      data array, which in fact is <code>timeLength/2+1</code>
         *      all the time.
         * 
         */
        void doFFT(const kiss_fft_scalar *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength);
        /**
         * @brief Performs a FFT on complex input values.
         * 
         * Uses kissfft.
         * 
         * @param timeData The data in the time domain. This is an array of a struct which is binary compatible with std::complex<double>.
         * @param timeLength The length of the time data array. Should be a power of 2.
         * @param freqData Returns the data in the frequency domain.
         * @param freqLength[out] returns the length of the frequency
         *      data array, which in fact is <code>timeLength/2+1</code>
         *      all the time.
         * 
         */
        void docFFT(const kiss_fft_cpx *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength);
        
        /**
         * @brief Performs an inverse FFT with real output values.
         * @bug does not report the lengths of the fields correctly
         */
        void doiFFT(const kiss_fft_cpx *freqData, int freqLength, kiss_fft_scalar *timeData, int& timeLength);
        
        // /**
        //  * @brief Performs an inverse FFT with complex output values.
        //  * @todo Implementation
        //  */
        // void doicFFT(const kiss_fft_cpx *freqData, int freqLength, kiss_fft_cpx *timeData, int& timeLength);
    };
    
    class DCT
    {
    private:
        FFT fft;
    protected:
        
    public:
        DCT();
        ~DCT();
        void doDCT1(float* timeData, int timeLength, float* freqData);
        void doDCT2(float* timeData, int timeLength, float* freqData);
    };
}

#endif //FFT_HPP

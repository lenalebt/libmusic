#ifndef FFT_HPP
#define FFT_HPP

#include "kiss_fftr.h"
#include "kiss_fft.h"

namespace music
{
    #define FFT_PREFACTOR_COUNT 12
    /**
     * @brief This class implements the Fast Fourier Transform.
     * 
     * This class actually is a wrapper for the kissfft library which
     * implements the FFT.
     * 
     * @ingroup transforms
     * @remarks Does not work in-place!
     * 
     * @author Lena Brueder
     * @date 2012-08-29
     */
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
}

#endif //FFT_HPP

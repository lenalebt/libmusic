#ifndef FFT_HPP
#define FFT_HPP

#include "kiss_fftr.h"

namespace music
{
    class FFT
    {
    private:
        
    public:
        /**
         * @brief Performs a FFT.
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
         * @remarks 
         * @todo Implementation
         */
        void doFFT(const kiss_fft_scalar *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength);
        /**
         * @brief Performs an inverse FFT.
         * @todo Implementation
         */
        void doiFFT(const kiss_fft_cpx *freqData, int freqLength, kiss_fft_scalar *timeData, int& timeLength);
    };
}

#endif //FFT_HPP

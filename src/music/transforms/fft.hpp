#ifndef FFT_HPP
#define FFT_HPP

#define kiss_fft_scalar float
typedef struct { kiss_fft_scalar r; kiss_fft_scalar i; }kiss_fft_cpx;

#include <fftw3.h>

namespace music
{
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
		fftwf_plan fftw_pc;
		fftwf_plan fftw_pr;
		float* fftw_inr;
		fftwf_complex* fftw_in;
		fftwf_complex* fftw_out;
		
		int fftLen;
    public:
        FFT(int size);
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
    };
}

#endif //FFT_HPP

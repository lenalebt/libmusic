#include "fft.hpp"
#include "debug.hpp"

namespace music
{
    FFT::FFT(int size) : fftLen(size)
    {
		fftw_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftLen);
		fftw_inr = (float*) fftwf_malloc(sizeof(float) * fftLen);
		fftw_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftLen);
		fftw_pc = fftwf_plan_dft_1d(fftLen, fftw_in, fftw_out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_pr = fftwf_plan_dft_r2c_1d(fftLen, fftw_inr, fftw_out, FFTW_ESTIMATE);
	}
    FFT::~FFT()
    {
		fftwf_destroy_plan(fftw_pc);
		fftwf_destroy_plan(fftw_pr);
        fftwf_free(fftw_in);
        fftwf_free(fftw_inr);
        fftwf_free(fftw_out);
    }
    
    void FFT::doFFT(const kiss_fft_scalar *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength)
    {
		for (int i = 0; i < fftLen; i++)
		{
			fftw_inr[i] = timeData[i];
		}
		
		fftwf_execute(fftw_pr);
		
		for (int i = 0; i < fftLen; i++)
		{
			freqData[i].r = fftw_out[i][0];
			freqData[i].i = fftw_out[i][1];
		}
		freqLength = timeLength/2+1;
    }
    
    void FFT::docFFT(const kiss_fft_cpx *timeData, int timeLength, kiss_fft_cpx *freqData, int& freqLength)
    {
		for (int i = 0; i < fftLen; i++)
		{
			fftw_in[i][0] = timeData[i].r;
			fftw_in[i][1] = timeData[i].i;
		}
		
		fftwf_execute(fftw_pc);
		
		for (int i = 0; i < fftLen; i++)
		{
			freqData[i].r = fftw_out[i][0];
			freqData[i].i = fftw_out[i][1];
		}
		freqLength = timeLength;
    }
}

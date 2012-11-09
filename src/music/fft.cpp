#include "fft.hpp"
#include "debug.hpp"

namespace music
{
    FFT::FFT()
    {
        for (int i=0; i<12; i++)
        {
            int timeLength = 1 << (i+1);
            //init kissfft
            rCfg[i] = kiss_fftr_alloc(timeLength, 0, NULL, NULL);
            cCfg[i] = kiss_fft_alloc(timeLength, 0, NULL, NULL);
            
            //init inverse kissfft
            riCfg[i] = kiss_fftr_alloc(timeLength, 1, NULL, NULL);
            ciCfg[i] = kiss_fft_alloc(timeLength, 1, NULL, NULL);
        }
    }
    FFT::~FFT()
    {
        for (int i=0; i<12; i++)
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
        for (i=0; i<12; i++)
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
        for (i=0; i<12; i++)
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
        for (i=0; i<12; i++)
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
        for (i=0; i<12; i++)
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
    
    void DCT::doDCT(float* timeData, int timeLength, float* freqData)
    {
        int n=timeLength-1;
        int freqLength;
        
        float sum, y1, y2, theta, wi=0.0, wpi, wpr, wr=1.0, wtemp;
        float* tmpData = new float[n];   //TODO: get rid of this part... dynamic memory allocation is uncool in terms of efficiency!
        
        theta = M_PI/n;
        DEBUG_VAR_OUT(theta, 0);
        wtemp = sin(0.5 * theta);
        DEBUG_VAR_OUT(wtemp, 0);
        wpr = -2.0 * wtemp*wtemp;
        DEBUG_VAR_OUT(wpr, 0);
        wpi = sin(theta);
        DEBUG_VAR_OUT(wpi, 0);
        sum = 0.5 * (timeData[0] - timeData[n]);
        DEBUG_VAR_OUT(sum, 0);
        tmpData[0] = 0.5 * (timeData[0] + timeData[n]);
        DEBUG_VAR_OUT(tmpData[0], 0);
        
        for (int j=1; j<n/2; j++)
        {
            wr = (wtemp=wr)*wpr - wi*wpi+wr;
            //DEBUG_VAR_OUT(wr, 0);
            wi = wi*wpr + wtemp*wpi + wi;
            //DEBUG_VAR_OUT(wi, 0);
            y1 = 0.5*(timeData[j] + timeData[n-j]);
            //DEBUG_VAR_OUT(y1, 0);
            y2 =     (timeData[j] - timeData[n-j]);
            //DEBUG_VAR_OUT(y2, 0);
            tmpData[j]      = y1 - wi*y2;
            //DEBUG_VAR_OUT(tmpData[j], 0);
            tmpData[n-j]    = y1 + wi*y2;
            //DEBUG_VAR_OUT(tmpData[n-j], 0);
            sum += wr*y2;
            //DEBUG_VAR_OUT(sum, 0);
        }
        tmpData[n/2] = timeData[n/2];
        
        for(int i=0; i<128; i++)
        {
            std::cerr << tmpData[i] << " ";
        }
        std::cerr << std::endl << std::endl;
        
        fft.doFFT(tmpData, n, (kiss_fft_cpx*)freqData, freqLength);
        DEBUG_OUT("freqLength: " << freqLength, 0);
        
        for(int i=0; i<128; i++)
        {
            std::cerr << freqData[i] << " ";
        }
        std::cerr << std::endl << std::endl;
        
        /*int midPoint = n/2;
        for (int i=1; i<fftLen/2; i++)
        {
            fftData[midPoint + i] = conj(fftData[midPoint - i]);
        }*/
        
        freqData[n] = freqData[1];
        freqData[1] = sum;
        for (int j=3; j<n; j+=2)
        {
            sum += freqData[j];
            freqData[j] = sum;
        }
        
        delete tmpData;
    }
}

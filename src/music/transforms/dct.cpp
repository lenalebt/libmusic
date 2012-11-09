#include "dct.hpp"

#include "debug.hpp"

namespace music
{
    
    void DCT::doDCT2(kiss_fft_scalar* timeData, int timeLength, kiss_fft_scalar* freqData)
    {
        
        for (int k=0; k<timeLength; k++)
        {
            double sum=0.0;
            for (int n=0; n<timeLength; n++)
            {
                sum += timeData[n] * cos(M_PI/double(timeLength) * (double(n)+0.5) * double(k));
            }
            freqData[k] = sum;
        }
        //the following code contains some errors, but it is fast. has been replaced by the above slower version, which is accurate.
        #if 0
        //taken from "Numerical Recipes, Third Edition"
        //imho, this code is veeeery ugly
        int n=timeLength;
        int freqLength;
        
        double sum, sum1, y1, y2, theta, wi=0.0, wi1, wpi, wpr, wr=1.0, wr1, wtemp;
        kiss_fft_scalar* tmpData = new kiss_fft_scalar[n];      //TODO: These two memory allocations are not inherently needed
        kiss_fft_scalar* tmpData2 = new kiss_fft_scalar[n+2];   //but I don't know for the moment how to fix it without affecting
                                            //too many other parts of the software.
        
        theta = 0.5*M_PI/n;
        wr1 = cos(theta);
        wi1 = sin(theta);
        wpr = -2.0 * wi1*wi1;
        wpi = sin(2.0 * theta);
        
        for (int i=0; i<n/2; i++)
        {
            y1 = 0.5 * (timeData[i] + timeData[n-1-i]);
            y2 = wi1 * (timeData[i] - timeData[n-1-i]);
            tmpData[i] =        y1 + y2;
            tmpData[n-1-i] =    y1 - y2;
            wr1 = (wtemp=wr1)*wpr - wi1*wpi + wr1;
            wi1 = wi1*wpr + wtemp*wpi + wi1;
        }
        
        fft.doFFT(tmpData, n, (kiss_fft_cpx*)tmpData2, freqLength);
        
        freqData[0] = tmpData2[0];
        for (int i=2; i<n; i+=2)
        {
            wr = (wtemp=wr)*wpr - wi*wpi + wr;
            wi = wi*wpr + wtemp*wpi + wi;
            y1 = tmpData2[i]*wr - tmpData2[i+1]*wi;
            y2 = tmpData2[i+1]*wr + tmpData2[i]*wi;
            freqData[i] = y1;
            freqData[i+1] = y2;
        }
        
        sum = 0.5 * tmpData2[1];
        for (int i=n-1; i>0; i-=2)
        {
            sum1 = sum;
            sum += freqData[i];
            freqData[i] = sum1;
        }
        
        delete[] tmpData;
        delete[] tmpData2;
        #endif
    }
    void DCT::doDCT1(kiss_fft_scalar* timeData, int timeLength, kiss_fft_scalar* freqData)
    {
        //taken from "Numerical Recipes, Third Edition"
        //imho, this code is veeeery ugly
        int n=timeLength-1;
        int freqLength;
        
        kiss_fft_scalar sum, y1, y2, theta, wi=0.0, wpi, wpr, wr=1.0, wtemp;
        kiss_fft_scalar* tmpData = new kiss_fft_scalar[n];   //TODO: get rid of this part... dynamic memory allocation is uncool in terms of efficiency!
        
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
        
        delete[] tmpData;
    }
    
    DCT::DCT() : fft()
    {
        
    }
    DCT::~DCT()
    {
        
    }
}

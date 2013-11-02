#include "filter.hpp"

#include <cstring>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <assert.h>

//0 means chebychef filter type 2 of order 10 (or 5),
//1 means butterworth filter of order 6.
#define IIR_FILTER_IMPLEMENTATION 1

namespace musicaccess
{
    IIRFilter::IIRFilter()
    {
        
    }
    IIRFilter::IIRFilter(const IIRFilter& filter) :
        A(filter.A), B(filter.B)
    {
        for (int i = 0; i < A; i++)
        {
            a[i] = filter.a[i];
        }
        for (int i = 0; i < B; i++)
        {
            b[i] = filter.b[i];
        }
    }

    IIRFilter* IIRFilter::createLowpassFilter(float relativeCutoff)
    {
        if (11025.0/22050.0 < relativeCutoff)
            return createLowpassFilter(11025, 22050);
        else if (11025.0/32000.0 <= relativeCutoff)
            return createLowpassFilter(11025, 32000);
        else if (11025.0/44100.0 <= relativeCutoff)
            return createLowpassFilter(11025, 44100);   //this one is used by the constant q transform.
        else if (11025.0/48000.0 <= relativeCutoff)
            return createLowpassFilter(11025, 48000);
        else if (11025.0/88200.0 <= relativeCutoff)
            return createLowpassFilter(11025, 88200);
        else if (11025.0/96000.0 <= relativeCutoff)
            return createLowpassFilter(11025, 96000);
        else
            return NULL;
    }

    IIRFilter* IIRFilter::createLowpassFilter(uint32_t cutoffFreq, uint32_t sampleFreq)
    {
        if (cutoffFreq != 11025)
        {
            std::cerr << "IIRFilter::createLowpassFilter(): only cutoff "
                << "frequency 11025Hz is supported for now, "
                << "will use that value instead of " << cutoffFreq << "."
                << std::endl;
            cutoffFreq = 11025;
        }
        IIRFilter* filter = NULL;
        filter = new IIRFilter();
        assert(filter != NULL);
        //std::cout << "sample freq:" << sampleFreq << std::endl;
        
#if IIR_FILTER_IMPLEMENTATION==0
        switch (sampleFreq)
        {
            //values computed with matlabs signal processing toolbox via call
            //[B, A] = cheby2(10, 30, 20000/96000)
            case 96000:
            {
                filter->A=11;
                filter->a[0] =  1.000000000000000e+000;
                filter->a[1] = -5.137912784424684e+000;
                filter->a[2] =  1.270349481112883e+001;
                filter->a[3] = -1.942711328063921e+001;
                filter->a[4] =  2.018965227928373e+001;
                filter->a[5] = -1.479753719251226e+001;
                filter->a[6] =  7.722465411597467e+000;
                filter->a[7] = -2.823426547617122e+000;
                filter->a[8] =  6.941387041771265e-001;
                filter->a[9] = -1.040950156204372e-001;
                filter->a[10] = 8.162383886257802e-003;
                
                filter->B=11;
                filter->b[0] =  3.663769108544628e-002;
                filter->b[1] = -1.314406324740274e-001;
                filter->b[2] =  2.671008430602078e-001;
                filter->b[3] = -3.507937288713959e-001;
                filter->b[4] =  3.721036162850466e-001;
                filter->b[5] = -3.593868089108566e-001;
                filter->b[6] =  3.721036162850466e-001;
                filter->b[7] = -3.507937288713963e-001;
                filter->b[8] =  2.671008430602084e-001;
                filter->b[9] = -1.314406324740280e-001;
                filter->b[10] = 3.663769108544647e-002;
                break;
            }
            //[B, A] = cheby2(10, 30, 20000/48000)
            case 48000:
            {
                filter->A=11;
                filter->a[0] =   1.000000000000000e+000;
                filter->a[1] =  -3.513395540449221e-001;
                filter->a[2] =   1.997612088363425e+000;
                filter->a[3] =   2.674224616948659e-002;
                filter->a[4] =   1.411590148523950e+000;
                filter->a[5] =   4.155654845994292e-001;
                filter->a[6] =   5.265305086936920e-001;
                filter->a[7] =   2.181882913259352e-001;
                filter->a[8] =   1.093754378305480e-001;
                filter->a[9] =   3.394228071728353e-002;
                filter->a[10] =  7.328484969337601e-003;
                
                filter->B=11;
                filter->b[0] =  8.548307236435304e-002;
                filter->b[1] =  1.859437159125497e-001;
                filter->b[2] =  4.299097241777818e-001;
                filter->b[3] =  6.549206148674901e-001;
                filter->b[4] =  8.707216429883506e-001;
                filter->b[5] =  9.415778765271075e-001;
                filter->b[6] =  8.707216429883496e-001;
                filter->b[7] =  6.549206148674884e-001;
                filter->b[8] =  4.299097241777803e-001;
                filter->b[9] =  1.859437159125489e-001;
                filter->b[10] = 8.548307236435254e-002;
                break;
            }
            //[B, A] = cheby2(10, 30, 20000/44100)
            //case 32000:
            case 44100:
            {
                filter->A=11;
                filter->a[0] =  1.000000000000000e+000;
                filter->a[1] =  4.756783834846750e-001;
                filter->a[2] =  2.142731349377693e+000;
                filter->a[3] =  1.355919751752093e+000;
                filter->a[4] =  1.920366793806582e+000;
                filter->a[5] =  1.222719734622602e+000;
                filter->a[6] =  8.922091320262994e-001;
                filter->a[7] =  4.470026948362085e-001;
                filter->a[8] =  1.894942685123683e-001;
                filter->a[9] =  5.598094821809568e-002;
                filter->a[10] = 9.903233008367150e-003;
                
                filter->B=11;
                filter->b[0] =  9.948268932339963e-002;
                filter->b[1] =  3.056825924563374e-001;
                filter->b[2] =  7.171101102954085e-001;
                filter->b[3] =  1.203387134907730e+000;
                filter->b[4] =  1.631943048581488e+000;
                filter->b[5] =  1.796795138516269e+000;
                filter->b[6] =  1.631943048581487e+000;
                filter->b[7] =  1.203387134907728e+000;
                filter->b[8] =  7.171101102954071e-001;
                filter->b[9] =  3.056825924563366e-001;
                filter->b[10] = 9.948268932339947e-002;
                break;
            }
            //[B, A] = cheby2(10, 30, 20000/32000)
            case 32000:
            {
                filter->A=11;
                filter->a[0] =  1.000000000000000e+000;  
                filter->a[1] =  4.162457158002721e+000;
                filter->a[2] =  9.669950334027194e+000;
                filter->a[3] =  1.515217762875054e+001;
                filter->a[4] =  1.738492075253834e+001;
                filter->a[5] =  1.503655566514470e+001;
                filter->a[6] =  9.856374437932093e+000;
                filter->a[7] =  4.813882183947667e+000;
                filter->a[8] =  1.674171664868208e+000;
                filter->a[9] =  3.748139534445399e-001;
                filter->a[10] = 4.120500265388572e-002;
                
                filter->B=11;
                filter->b[0] =  2.029901364266410e-001;
                filter->b[1] =  1.345701236157712e+000;
                filter->b[2] =  4.487547697518863e+000;
                filter->b[3] =  9.738254085864973e+000;
                filter->b[4] =  1.510177506652912e+001;
                filter->b[5] =  1.741397233631534e+001;
                filter->b[6] =  1.510177506652913e+001;
                filter->b[7] =  9.738254085864991e+000;
                filter->b[8] =  4.487547697518876e+000;
                filter->b[9] =  1.345701236157717e+000;
                filter->b[10] = 2.029901364266419e-001;
                break;
            }
            //[B, A] = cheby2(5, 30, 20000/22050)   //cheby(10, ..., ...) is instable, algorithm is not robust
            case 22050:
            {
                filter->A=6;
                filter->a[0] = 1.000000000000000e+000;  
                filter->a[1] = 4.035642697260309e+000;
                filter->a[2] = 6.661664165574882e+000;
                filter->a[3] = 5.604686380760152e+000;
                filter->a[4] = 2.398408916209979e+000;
                filter->a[5] = 4.170082635280277e-001;
                
                filter->B=6;
                filter->b[0] = 6.457617756083814e-001;
                filter->b[1] = 3.160070284959489e+000;
                filter->b[2] = 6.252873151098805e+000;
                filter->b[3] = 6.252873151098805e+000;
                filter->b[4] = 3.160070284959489e+000;
                filter->b[5] = 6.457617756083813e-001;
                break;
            }
            default:
            {
                std::cerr << "target sample frequency not available: "
                    << sampleFreq << std::endl;
                break;
            }
        }
#elif IIR_FILTER_IMPLEMENTATION==1
        switch (sampleFreq)
        {
            //values computed with matlabs signal processing toolbox via call
            //[B, A] = butter(6, 20000/96000)
            case 96000:
            {
                filter->A=7;
                filter->a[0] =  1.000000000000000e+000;
                filter->a[1] = -3.479137768356288e+000;
                filter->a[2] =  5.383731201376863e+000;
                filter->a[3] = -4.640607571636227e+000;
                filter->a[4] =  2.328230772635863e+000;
                filter->a[5] = -6.405433446158334e-001;
                filter->a[6] =  7.517403174874351e-002;
                
                filter->B=7;
                filter->b[0] = 4.194893930175164e-004;
                filter->b[1] = 2.516936358105099e-003;
                filter->b[2] = 6.292340895262746e-003;
                filter->b[3] = 8.389787860350328e-003;
                filter->b[4] = 6.292340895262746e-003;
                filter->b[5] = 2.516936358105099e-003;
                filter->b[6] = 4.194893930175164e-004;
                break;
            }
            //[B, A] = butter(6, 20000/48000)
            case 48000:
            {
                filter->A=7;
                filter->a[0] =   1.000000000000000e+000;
                filter->a[1] =  -9.894650827516605e-001;
                filter->a[2] =   1.143622537508783e+000;
                filter->a[3] =  -5.327615713085703e-001;
                filter->a[4] =   2.150444144228833e-001;
                filter->a[5] =  -3.935997039939775e-002;
                filter->a[6] =   3.916187439517707e-003;
                
                filter->B=7;
                filter->b[0] = 1.251557054549306e-002;
                filter->b[1] = 7.509342327295838e-002;
                filter->b[2] = 1.877335581823960e-001;
                filter->b[3] = 2.503114109098613e-001;
                filter->b[4] = 1.877335581823960e-001;
                filter->b[5] = 7.509342327295838e-002;
                filter->b[6] = 1.251557054549306e-002;
                break;
            }
            //[B, A] = butter(6, 20000/41000)
            //case 32000:
            case 44100:
            {
                filter->A=7;
                filter->a[0] =   1.000000000000000e+000;
                filter->a[1] =  -1.447329639371992e-001;
                filter->a[2] =   7.855132308451324e-001;
                filter->a[3] =  -6.854573002656453e-002;
                filter->a[4] =   1.162268601653623e-001;
                filter->a[5] =  -4.600818555709145e-003;
                filter->a[6] =   1.792435332432211e-003;
                
                filter->B=7;
                filter->b[0] =  2.633832834099147e-002;
                filter->b[1] =  1.580299700459488e-001;
                filter->b[2] =  3.950749251148720e-001;
                filter->b[3] =  5.267665668198294e-001;
                filter->b[4] =  3.950749251148720e-001;
                filter->b[5] =  1.580299700459488e-001;
                filter->b[6] =  2.633832834099147e-002;
                break;
            }
            //[B, A] = butter(6, 20000/32000)
            case 32000:
            {
                filter->A=7;
                filter->a[0] =  1.000000000000000e+000;  
                filter->a[1] =  1.485051528776589e+000;
                filter->a[2] =  1.603614295818288e+000;
                filter->a[3] =  9.240601089009823e-001;
                filter->a[4] =  3.592332587432241e-001;
                filter->a[5] =  7.561117796037330e-002;
                filter->a[6] =  7.322146251062951e-003;
                
                filter->B=7;
                filter->b[0] = 8.523269556953936e-002;
                filter->b[1] = 5.113961734172361e-001;
                filter->b[2] = 1.278490433543090e+000;
                filter->b[3] = 1.704653911390787e+000;
                filter->b[4] = 1.278490433543090e+000;
                filter->b[5] = 5.113961734172361e-001;
                filter->b[6] = 8.523269556953936e-002;
                break;
            }
            //[B, A] = butter(6, 20000/22050)
            case 22050:
            {
                /*
                filter->A=6;
                filter->a[0] = 1.000000000000000e+000;  
                filter->a[1] = 4.872275990105688e+000;
                filter->a[2] = 9.979812734259321e+000;
                filter->a[3] = 1.098805324243322e+001;
                filter->a[4] = 6.853204620014873e+000;
                filter->a[5] = 2.294188838222453e+000;
                filter->a[6] = 3.218683601988168e-001;
                               
                filter->B=6;
                filter->b[0] = 5.673344341442871e-001;
                filter->b[1] = 3.404006604865723e+000;
                filter->b[2] = 8.510016512164306e+000;
                filter->b[3] = 1.134668868288574e+001;
                filter->b[4] = 8.510016512164306e+000;
                filter->b[5] = 3.404006604865723e+000;
                filter->b[6] = 5.673344341442871e-001;
                */
                filter->A = 0;
                filter->b[0] = 1.0f;
                filter->B = 1;
                
                break;
            }
            default:
            {
                std::cerr << "target sample frequency not available: "
                    << sampleFreq << std::endl;
                break;
            }
        }
#endif //IIR_FILTER_IMPLEMENTATION
        
        return filter;
    }
    IIRFilter* IIRFilter::createNOOPFilter()
    {
        IIRFilter* filter = new IIRFilter();
        filter->A = 0;
        filter->b[0] = 1.0f;
        filter->B = 1;
        return filter;
    }

    void IIRFilter::apply(int16_t* buffer, int bufferSize) const
    {
        //applies an IIR filter in-place.
        
        int16_t history[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        for (int i = 0; i < MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT; i++)
        {
            history[i] = 0;
        }
        
        int historyPos=0;
        for (int i = 0; i < bufferSize; i++, historyPos++)
        {
            if (historyPos >= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT)
                historyPos -= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT;
            
            //save history, we need it because of the recursive structure (input values will be overwritten)
            history[historyPos] = buffer[i];
            
            iirfilter_coefficienttype tmpVal=0.0;
            for (int l = 1; l < A; l++)
            {
                tmpVal -= a[l] * ((i-l<0) ? 0.0 : (iirfilter_coefficienttype(buffer[i - l])));
            }
            for (int k = 0; k < B; k++)
            {
                //this is b[k] * x[i-k], written in terms of the history of our MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT most recent input samples
                tmpVal += b[k] * (iirfilter_coefficienttype(history[(historyPos - k + MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT) % MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT]));
            }
            buffer[i] = int16_t(std::floor(tmpVal+0.5));
        }
    }
    void IIRFilter::apply(float* buffer, int bufferSize) const
    {
        //applies an IIR filter in-place.
        
        float history[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        for (int i = 0; i < MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT; i++)
        {
            history[i] = 0;
        }
        
        int historyPos=0;
        for (int i = 0; i < bufferSize; i++, historyPos++)
        {
            if (historyPos >= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT)
                historyPos -= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT;
            
            //save history, we need it because of the recursive structure (input values will be overwritten)
            history[historyPos] = buffer[i];
            
            iirfilter_coefficienttype tmpVal=0.0;
            for (int l = 1; l < A; l++)
            {
                tmpVal -= a[l] * ((i-l<0) ? 0.0 : (iirfilter_coefficienttype(buffer[i - l])));
            }
            for (int k = 0; k < B; k++)
            {
                //this is b[k] * x[i-k], written in terms of the history of our MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT most recent input samples
                tmpVal += b[k] * (iirfilter_coefficienttype(history[(historyPos - k + MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT) % MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT]));
            }
            buffer[i] = tmpVal;
        }
    }
    
    SortingIIRFilter::SortingIIRFilter()
    {
        
    }
    void SortingIIRFilter::apply(int16_t* buffer, int bufferSize) const
    {
        //applies an IIR filter in-place, in order of growing absolute values of coefficients.
        
        int16_t history[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        for (int i = 0; i < MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT; i++)
        {
            history[i] = 0;
        }
        
        int historyPos=0;
        for (int i = 0; i < bufferSize; i++, historyPos++)
        {
            if (historyPos >= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT)
                historyPos -= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT;
            
            //save history, we need it because of the recursive structure (input values will be overwritten)
            history[historyPos] = buffer[i];
            
            iirfilter_coefficienttype tmpVal=0.0;
            for (int l = 0; l < A; l++)
            {
                if (aOrder[l] == 0)
                    continue;   //workaround for skipping a[0]
                tmpVal -= a[aOrder[l]] * ((i-aOrder[l]<0) ? 0.0 : (iirfilter_coefficienttype(buffer[i - aOrder[l]])));
            }
            for (int k = 0; k < B; k++)
            {
                //this is b[k] * x[i-k], written in terms of the history of our MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT most recent input samples
                tmpVal += b[bOrder[k]] * (iirfilter_coefficienttype(history[(historyPos - bOrder[k] + MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT) % MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT]));
            }
            buffer[i] = int16_t(std::floor(tmpVal+0.5));
        }
    }
    void SortingIIRFilter::apply(float* buffer, int bufferSize) const
    {
        //applies an IIR filter in-place, in order of growing absolute values of coefficients.
        
        float history[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        for (int i = 0; i < MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT; i++)
        {
            history[i] = 0;
        }
        
        int historyPos=0;
        for (int i = 0; i < bufferSize; i++, historyPos++)
        {
            if (historyPos >= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT)
                historyPos -= MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT;
            
            //save history, we need it because of the recursive structure (input values will be overwritten)
            history[historyPos] = buffer[i];
            
            iirfilter_coefficienttype tmpVal=0.0;
            for (int l = 0; l < A; l++)
            {
                if (aOrder[l] == 0)
                    continue;   //workaround for skipping a[0]
                tmpVal -= a[aOrder[l]] * ((i-aOrder[l]<0) ? 0.0 : (iirfilter_coefficienttype(buffer[i - aOrder[l]])));
            }
            for (int k = 0; k < B; k++)
            {
                //this is b[k] * x[i-k], written in terms of the history of our MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT most recent input samples
                tmpVal += b[bOrder[k]] * (iirfilter_coefficienttype(history[(historyPos - bOrder[k] + MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT) % MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT]));
            }
            buffer[i] = tmpVal;
        }
    }
    bool SortingIIRFilter::pairFirstElementComparator(const std::pair<iirfilter_coefficienttype, int>& a, const std::pair<iirfilter_coefficienttype, int>& b)
    {
        return a.first < b.first;
    }
    void SortingIIRFilter::sortCoefficients()
    {
        std::pair<iirfilter_coefficienttype, int> coefficientsA[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        std::pair<iirfilter_coefficienttype, int> coefficientsB[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        
        //copy data to pair array
        for (int i=0; i<A; i++)
        {
            coefficientsA[i].first = std::fabs(a[i]);
            coefficientsA[i].second = i;
        }
        for (int i=0; i<B; i++)
        {
            coefficientsB[i].first = std::fabs(b[i]);
            coefficientsB[i].second = i;
        }
        
        //sort pair arrays based on first element (->absolute value)
        //for A, the first element does not count... (no sorting here)
        std::sort(coefficientsA, coefficientsA+A, SortingIIRFilter::pairFirstElementComparator);
        std::sort(coefficientsB  , coefficientsB+B, SortingIIRFilter::pairFirstElementComparator);
        
        //get indexes back. use them while applying the filter.
        for (int i=0; i<A; i++)
        {
            aOrder[i] = coefficientsA[i].second;
        }
        for (int i=0; i<B; i++)
        {
            bOrder[i] = coefficientsB[i].second;
        }
    }
    SortingIIRFilter* SortingIIRFilter::createFilter(const IIRFilter& filter)
    {
        SortingIIRFilter* sortFilter = new SortingIIRFilter();
        std::memcpy(sortFilter->a, filter.a, MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT*sizeof(iirfilter_coefficienttype));
        std::memcpy(sortFilter->b, filter.b, MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT*sizeof(iirfilter_coefficienttype));
        sortFilter->A = filter.A;
        sortFilter->B = filter.B;
        
        for (int i=0; i<MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT; i++)
        {
            sortFilter->aOrder[i] = i;
            sortFilter->bOrder[i] = i;
        }
        
        sortFilter->sortCoefficients();
        
        return sortFilter;
    }
}

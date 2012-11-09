#ifndef DCT_HPP
#define DCT_HPP

#include "fft.hpp"

namespace music
{
    /**
     * @todo DCT documentation
     */
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
#endif  //DCT_HPP

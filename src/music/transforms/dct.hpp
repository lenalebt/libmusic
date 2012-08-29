#ifndef DCT_HPP
#define DCT_HPP

#include "fft.hpp"

/**
 * @defgroup transforms Time/Frequency Transforms
 * @brief This group contains all transforms from time to the frequency
 *      domain (and vice versa, if applicable) that were used in the
 *      project
 */

namespace music
{
    /**
     * @brief An implementation of a discrete cosine transform.
     * 
     * The Discrete Cosine Transform is related to the Discrete Fourier
     * Transform. Both switch from the time- to the frequency domain.
     * The DCT is a transform from real values to real values - in
     * contrast to the DFT, which transforms complex or real values to
     * complex values.
     * 
     * @ingroup transforms
     * @remarks The transform could be improved regarding speed.
     * 
     * @author Lena Brueder
     * @date 2012-08-29
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

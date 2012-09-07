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
     * @bug The DCT algorithms were more or less
     *      taken from the book "Numerical Recipes".
     *      The algorithms in this book are not free software! These algorithms
     *      should be replaced soon.
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
            /**
             * @brief Performs a discrete cosine transform, form I.
             * 
             * @remarks This transform is its own reverse. If you use it as
             *      reverse, you need to multiply the results with
             *      <code>1.0/timeLength</code>.
             * 
             * @param[in] timeData The data in the time domain that should be
             *      transformed.
             * @param[in] timeLength The number of samples in the time domain.
             * @param[out] freqData The data in the frequency domain. You need
             *      to supply the function with a pointer to at least
             *      <code>timeLength</code> <code>float</code>s.
             */
            void doDCT1(float* timeData, int timeLength, float* freqData);
            /**
             * @brief Performs a discrete cosine transform, form II.
             * 
             * @remarks To calculate the inverse of this transform, you need
             *      another routine (not implemented). See "Numerical Recipes",
             *      Third edition, Chapter 12, "Cosine Transform" (page 626).
             * 
             * @param[in] timeData The data in the time domain that should be
             *      transformed.
             * @param[in] timeLength The number of samples in the time domain.
             * @param[out] freqData The data in the frequency domain. You need
             *      to supply the function with a pointer to at least
             *      <code>timeLength</code> <code>float</code>s.
             */
            void doDCT2(float* timeData, int timeLength, float* freqData);
        };
}
#endif  //DCT_HPP

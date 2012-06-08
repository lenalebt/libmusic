#ifndef CONSTANT_Q_HPP
#define CONSTANT_Q_HPP

#include "musicaccess/filter.hpp"

namespace music
{
    class ConstantQTransform
    {
    private:
        ConstantQTransform();
        inline double window(int width, int position);
    public:
        /**
         * @brief Creates the kernels for the Constant Q transform which can later be applied to many pieces of music.
         * 
         * This constant Q transform implementation uses the algorithm as presented in 
@verbatim
Christian Schörkhuber and Anssi Klapuri. Constant-Q transform toolbox for
music processing. In 7th Sound and Music Computing Conference, Barcelona,
Spain, 2010.
@endverbatim
         * . Essentially, it calculates the transform in form of a sparse matrix that can later be applied to
         * any input data, so the transform only needs to be calculated once, and can then be
         * applied many times.
         * 
         * @param binsPerOctave the number of frequency bins per octave
         * @param fMin the minimal frequency in Hz that is of interest
         * @param fMax the maximal frequency in Hz that is of interest
         * @param fs the sampling frequency of the audio data that will be processed in Hz (typically 22050 Hz)
         * @param lowpassFilter a lowpassfilter that will be applied during the process. Make sure that this filter has a cutoff frequency of fs/2.
         * @param q the q value, which is kind of the "size" of a bin. I is defined as the quotient of the bin base frequency and its bandwidth, which should stay constant.
         * @param threshold values in the CQT kernel smaller than this value will be vanished. use a larger value to gain accuracy, or a smaller value to gain speed.
         * @return A Constant Q Transform kernel that can be applied to a piece of music via its apply() function.
         * 
         * @todo implement
         */
        static ConstantQTransform* createTransform(int binsPerOctave, int fMin, int fMax, int fs, musicaccess::IIRFilter* lowpassFilter, float q=1.0f, float threshold=0.0005);
        /**
         * @brief Apply this constant Q transform to a given sound buffer.
         * 
         * 
         * 
         * @param buffer the sound buffer in 16bit signed integer format.
         * @param sampleCount the sample count
         * @return 
         * 
         * @remarks The sample rate must be set during creation of the transform. If you
         *      need to be able to apply a transform to music with different sample
         * rates, you need to create multiple transforms and then apply the right one.
         * @todo implement
         * @todo set the right return value.
         */
        void apply(uint16_t* buffer, int sampleCount);
    };
}
#endif //CONSTANT_Q_HPP

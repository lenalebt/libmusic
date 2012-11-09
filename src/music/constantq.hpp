#ifndef CONSTANT_Q_HPP
#define CONSTANT_Q_HPP

#include <musicaccess/filter.hpp>
#include "tests.hpp"
#include "fft.hpp"
#include <cmath>

#include <complex>
#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET
#include <Eigen/Sparse>
#include <Eigen/Dense>

/**
 * @page midinote_scale Midi note scale
 * To identify a musical note, the midi note scale ist used. Midi notes are just
 * semitones one after each other.
 * 
 * - The lowest note on a piano with 88 keys
 * is A0, which corresponds to midi note <code>21</code> (<code>0x15</code>).
 * In Germany, this note is called A''.
 * 
 * - The note for standard pitch of 440Hz is
 * A4, a' or midi note <code>69</code> (<code>0x55</code>).
 * 
 * - The lowest midi note is
 * <code>0</code> (<code>0x00</code>), which corresponds to C-1 or C''''.
 * 
 * - The highest midi note is
 * <code>127</code> (<code>0x7F</code>), which is G9 or g'''''.
 * 
 */

namespace music
{
    /**
     * @brief This class describes a result of a constant Q transform.
     * 
     * You are able to get the value of the constant Q transform at a
     * selected sample number. There are some different possibilities
     * to access the values.
     * 
     * @todo implement
     * 
     * @author Lena Brueder
     * @date 2012-06-12
     */
    class ConstantQTransformResult
    {
    private:
        int octaveCount;    //how many octaves did we process?
        int maxOctave;      //highest octave Octave
        //array of matricies. we have one matrix for every octave.
        //the matricies are dense, with one row being an octave bin.    TODO: is that right?
        Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic >* octaveMatrix;
    public:
        /**
         * @brief Returns the value of the constant Q transform at the given sample number
         *      and interpolates linearly between samples where the transform is known.
         * 
         * 
         * @param sampleNumber the sample number in time
         * @param midiNoteNumber the number of the midi note you want to get.
         * @see 
         * @return the value of the constant Q transform at a given sample number
         */
        std::complex<float> getNoteValueLinearInterpolation(int64_t sampleNumber, int midiNoteNumber);
        
        /**
         * @brief Returns the value of the constant Q transform at the given sample number
         *      and does not interpolate.
         * 
         * It just takes the value of the transform that is before the given sample number.
         * 
         * @return the value of the constant Q transform at a given sample number
         */
        std::complex<float> getNoteValueNoInterpolation(int64_t sampleNumber, int midiNoteNumber);
    };
    
    /**
     * @brief This class is capable of applying a constant Q transform
     *  to an input signal.
     * 
     * 
     * 
     * @todo unstable, api is subject to change.
     * @todo implementation is not ready yet.
     * 
     * @author Lena Brueder
     * @date 2012-06-12
     */
    class ConstantQTransform
    {
    private:
        int octaveCount;        //how many octaves are processed by this transform?
        int fMin;
        double kernelfMin;
        int fMax;
        int fs;
        int binsPerOctave;
        musicaccess::IIRFilter* lowpassFilter;
        double q;
        double Q;
        double threshold;
        double atomHopFactor;
        int fftHop;
        int fftLen;
        
        int nkMax;      //length of the largest atom in samples
        
        Eigen::SparseMatrix<std::complex<kiss_fft_scalar> >* fKernel;  //the transform kernel for one octave
        
        ConstantQTransform();
        //square root of blackman-harris window, as used in the matlab implementation of the mentioned paper. other window that might be okay: blackman.
        //coefficients taken from Wikipedia (permanent link to used article version): http://en.wikipedia.org/w/index.php?title=Window_function&oldid=495970218#Blackman.E2.80.93Harris_window
        //coefficients are identical to the mentioned values in the matlab documentation ("doc blackmanharris").
        template <typename T>
        static inline T window(int width, int position)
        {
            if ((position < 0) || (position > width))
                return 0.0;
            else
                return sqrt(0.35875 - 0.48829*cos(2*M_PI*position/(width-1)) + 0.14128*cos(4*M_PI*position/(width-1)) - 0.01168*cos(6*M_PI*position/(width-1)));
        }
        static inline double log2(double x) {return std::log(x) / std::log(2.0); /*compiler should optimize this at compile time*/}
    public:
        /**
         * @brief Returns the number of octaves that will be processed by this constant Q transform.
         * @return the number of octaves
         */
        int getOctaveCount() {return octaveCount;}
        /**
         * @brief Returns the frequency of the lowest tone that will be processed by this transform.
         * @return the frequency of the lowest tone
         */
        int getFMin() {return fMin;}
        /**
         * @brief Returns the frequency of the lowest tone in the highest octave that will be processed by this transform.
         * @return the frequency of the lowest tone
         */
        double getKernelFMin() {return kernelfMin;}
        /**
         * @brief Returns the frequency of the highest tone that will be processed by this transform.
         * @return the frequency of the highest tone
         */
        int getFMax() {return fMax;}
        /**
         * @brief Returns the sampling frequency that can be processed by this transform.
         * @return the possible sampling frequency of the audio that will be processed.
         */
        int getFs() {return fs;}
        /**
         * @brief Returns the count of the frequency bins that will be created by this transform.
         * @return the count of the frequency bins per ocatve
         */
        int getBinsPerOctave() {return binsPerOctave;}
        /**
         * @brief Returns the lowpass filter that is being used while transforming the input data.
         * @return the frequency of the lowest tone
         */
        const musicaccess::IIRFilter* getLowpassFilter() {return lowpassFilter;}
        /**
         * @brief Returns the Q factor used in this transform.
         * @remarks don't confuse q and Q. They are different.
         * @return the Q factor
         */
        double getQ() {return Q;}
        /**
         * @brief Returns the Q scaling factor (q) used in this transform.
         * @remarks don't confuse q and Q. They are different.
         * @return the Q scaling factor q
         */
        double getq() {return q;}
        /**
         * @brief Returns the threshold at which values in the spectral kernel will be seen as zero in this transform.
         * @return the threshold of values being treated as zero
         */
        double getThreshold() {return threshold;}
        /**
         * @todo write description
         */
        double getAtomHopFactor() {return atomHopFactor;}
        
        /**
         * @brief Returns the length of the FFT to apply to the input data
         * @return the length of the FFT to apply to the input data. Is a power of 2.
         */
        int getFFTLength() {return fftLen;}
        
        /**
         * @brief Returns the hop size for the FFTs on the input data.
         * 
         * Use this value to shift your FFT windows on the input data.
         * 
         * @return the hop size for the FFTs on the input data.
         */
        int getFFTHop() {return fftHop;}
        
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
         * @param binsPerOctave the number of frequency bins per octave. Typically, 12 will be used with western music.
         * @param fMin the minimal frequency in Hz that is of interest. This value will internally be recalculated such that we always take a look at whole octaves.
         * @param fMax the maximal frequency in Hz that is of interest
         * @param fs the sampling frequency of the audio data that will be processed in Hz (typically 22050 Hz)
         * @param lowpassFilter a lowpassfilter that will be applied during the process. Make sure that this filter has a cutoff frequency of fs/2.
         * @param q the q value, which is kind of the "size" of a bin. I is defined as the quotient of the bin base frequency and its bandwidth, which should stay constant.
         * @param threshold values in the CQT kernel smaller than this value will be vanished. use a larger value to gain accuracy, or a smaller value to gain speed.
         * @return A Constant Q Transform kernel that can be applied to a piece of music via its apply() function.
         * 
         * @todo implement
         */
        static ConstantQTransform* createTransform(musicaccess::IIRFilter* lowpassFilter, int binsPerOctave=12, int fMin=40, int fMax=11025, int fs=22050, double q=1.0, double threshold=0.0005, double atomHopFactor=0.25);
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
        ConstantQTransformResult* apply(uint16_t* buffer, int sampleCount);
        
        friend int tests::testConstantQ();
    };
}
#endif //CONSTANT_Q_HPP

#ifndef FILTER_HPP
#define FILTER_HPP

#include <stdint.h>
#include <utility>

namespace musicaccess
{
    const int MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT=64;
    typedef double iirfilter_coefficienttype;
    
    /**
     * @brief This class defines a simple audio filter.
     * 
     * The filter can be applied to a given buffer.
     * 
     * This is a pure virtual class, derive your own filters from this class.
     * 
     * @author Lena Brueder
     * @date 2012-05-21
     */
    class AudioFilter
    {
    private:
        
    public:
        /**
         * @brief Apply this filter to the given buffer with the given size.
         */
        virtual void apply(int16_t* buffer, int bufferSize) const=0;
        
        /**
         * @brief Apply this filter to the given buffer with the given size.
         */
        virtual void apply(float* buffer, int bufferSize) const=0;
    };

    /**
     * @brief This class defines an IIR filter implementation.
     * 
     * If you want some implementation you can use, take the factory methods to get
     * one. There are some implementations given for low-pass filters.
     * 
     * 
     * @author Lena Brueder
     * @date 2012-05-21
     */
    class IIRFilter : public AudioFilter
    {
    private:
        
    protected:
        IIRFilter();
        //coefficients for input values
        iirfilter_coefficienttype a[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        //coefficients for feedback values
        iirfilter_coefficienttype b[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        //input coefficient count
        int A;
        //feedback coefficient count
        int B;
    public:
        IIRFilter(const IIRFilter& filter);
        /**
         * @brief Applies the given IIR filter to the input buffer in-place.
         * 
         * @remarks The function internally uses a history buffer of <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> samples, so
         *      you will not be able to use more than <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> coefficients.
         * @remarks The given buffer will be overwritten to gain some speed. If
         *      you need the data in that buffer for other purposes, go make a copy!
         */
        void apply(int16_t* buffer, int bufferSize) const;
        
        /**
         * @brief Applies the given IIR filter to the input buffer in-place.
         * 
         * @remarks The function internally uses a history buffer of <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> samples, so
         *      you will not be able to use more than <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> coefficients.
         * @remarks The given buffer will be overwritten to gain some speed. If
         *      you need the data in that buffer for other purposes, go make a copy!
         */
        void apply(float* buffer, int bufferSize) const;
        
        /**
         * @brief Creates a lowpass filter with given cutoff frequency at the given
         *  sample rate.
         * 
         * This function only works for some standard sample rates (22050, 32000,
         * 44100, 48000 and 96000Hz) and cutoff frequency 11025Hz, and creates a Chebyshev
         * filter of order 10 or a Butterworth filter of order 6 - that depends on the
         * IIR_FILTER_IMPLEMENTATION compile time flag (currently to be set by hand in
         * the implementation file). Standard setting is a Butterworth filter.
         * 
         * @return an IIR filter that does lowpass filtering.
         */
        static IIRFilter* createLowpassFilter(uint32_t cutoffFreq, uint32_t sampleFreq);
        
        /**
         * @brief Creates a lowpass filter with given cutoff frequency relative to the
         *  sample rate.
         * 
         * This function uses precomputed filter coefficients. It always uses the
         * best-fitting coefficients which hold the relative cutoff
         * constraint. This might lead to more frequencies being cut off than
         * necessary.
         * 
         * @remarks This function might create a filter that cuts off a
         *      bit more than was needed or intended.
         * 
         * @return an IIR filter that does lowpass filtering.
         */
        static IIRFilter* createLowpassFilter(float relativeCutoff);
        /**
         * @brief Creates a IIR filter that exactly does nothing.
         * 
         * This function is used mainly for testing. The created
         * filter is a no-op filter.
         * 
         * @return An IIR filter that does not change the input signal.
         */
        static IIRFilter* createNOOPFilter();
        
        friend class SortingIIRFilter;
    };
    
    /**
     * @brief This class defines an IIR filter implementation.
     * 
     * If you want some implementation you can use, take the factory
     * methods to get one.
     * 
     * @author Lena Brueder
     * @date 2012-05-21
     */
    class SortingIIRFilter : public IIRFilter
    {
    private:
        
    protected:
        SortingIIRFilter();
        //the order of how the elements should be applied
        int aOrder[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        int bOrder[MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT];
        
        void sortCoefficients();
        
        static bool pairFirstElementComparator(const std::pair<iirfilter_coefficienttype, int>& a, const std::pair<iirfilter_coefficienttype, int>& b);
    public:
        /**
         * @brief Applies the given IIR filter to the input buffer in-place.
         * 
         * @remarks The function internally uses a history buffer of <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> samples, so
         *      you will not be able to use more than <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> coefficients.
         * @remarks The given buffer will be overwritten to gain some speed. If
         *      you need the data in that buffer for other purposes, go make a copy!
         */
        void apply(int16_t* buffer, int bufferSize) const;
        
        /**
         * @brief Applies the given IIR filter to the input buffer in-place.
         * 
         * @remarks The function internally uses a history buffer of <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> samples, so
         *      you will not be able to use more than <code>MUSICACCESS_IIRFILTER_COEFFICIENTCOUNT</code> coefficients.
         * @remarks The given buffer will be overwritten to gain some speed. If
         *      you need the data in that buffer for other purposes, go make a copy!
         */
        void apply(float* buffer, int bufferSize) const;
        /**
         * @brief Creates an iir filter with better numeric stability
         *      from of a given iir filter.
         * 
         * In detail, the new filter will use exactly the same coefficients
         * the old filter uses, but apply them in a different order.
         * Doing so, we should gain better stability and accuracy
         * (tests did not show that it helps, although in theory it should).
         * The filter coefficients will be sorted and added up in the
         * order of ascending absolute values.
         */
        static SortingIIRFilter* createFilter(const IIRFilter& filter);
    };
}

#endif //FILTER_HPP

#ifndef RANDOMNUMBERS_HPP
#define RANDOMNUMBERS_HPP

/**
 * @defgroup tools Tools
 * @brief Classes that are somehow useful for every part of the project will be placed here.
 */


#include <cstdlib>
namespace music
{
    /**
     * @brief A random number generator interface.
     * 
     * @ingroup tools
     * @tparam T The type of numbers that will be drawn.
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename T>
    class StandardRNG
    {
    public:
        /**
         * @brief Draw a random number from the distribution.
         * @return the random number.
         */
        virtual T rand() const=0;
    };
    
    /**
     * @brief A random number generator which produces uniformly distributed random numbers.
     * 
     * It draws values from the interval <code>[a, b[</code>, where <code>a</code> and
     * <code>b</code> can be chosen.
     * 
     * The interval might be closed for integer values, due to rounding errors.
     * 
     * @ingroup tools
     * @tparam T The type of numbers that will be drawn.
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename T>
    class UniformRNG : public StandardRNG<T>
    {
    private:
        
    protected:
        T a;    //lower bound of interval
        T b;    //upper bound of interval
        T diff; //b-a, stored to save CPU cycles.
    public:
        /**
         * @brief Initializes a uniform random number generator which draws values from <code>[0, 1[</code>.
         */
        UniformRNG() :
            a(0.0), b(1.0)  {diff = b-a;}
        /**
         * @brief Initializes a uniform random number generator which draws values from <code>[a, b[</code>.
         */
        UniformRNG(T a, T b) :
            a(a), b(b)      {diff = b-a;}
        T rand() const
        {
            return a + (diff)*(double(std::rand()) / RAND_MAX);
        }
        
        /**
         * @brief Returns the lower bound of the interval
         * @return the lower bound of the interval
         */
        T getA() const   {return a;}
        /**
         * @brief Returns the upper bound of the interval
         * @return the upper bound of the interval
         */
        T getB() const   {return b;}
    };
    
    /**
     * @brief A random number generator which produces normally distributed numbers.
     * 
     * @attention This class is not thread-safe. If you want to use it from multiple threads, you need to
     *      create one object per thread.
     * @ingroup tools
     * @tparam T The type of numbers that will be drawn.
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename T>
    class NormalRNG : public StandardRNG<T>
    {
    private:
        mutable double storedval;
    protected:
        UniformRNG<T>* rng;
        T mean;
        T variance;
    public:
        /**
         * @brief Construct a new random number generator with normally distributed samples.
         * 
         * @param rng The uniform random number generator which will be used to
         *      build the normal distribution. This object takes ownership of the rng!
         *      The uniform random number generator is assumed to draw values from [-1,1[.
         * @param mean The mean of the values produced by this random number generator
         * @param variance The variance of the values produced by this random number generator
         */
        NormalRNG(UniformRNG<T>* rng, T mean, T variance) :
            storedval(0.0), rng(rng), mean(mean), variance(variance)
        {
            if ((rng == NULL) || (rng->getA() != -1) || (rng->getB() != 1))
                rng = new UniformRNG<T>(-1, 1);
        }
        /**
         * @brief Construct a new random number generator with normally distributed samples.
         * 
         * This implementation uses a standard random number generator.
         * 
         * @param mean The mean of the values produced by this random number generator
         * @param variance The variance of the values produced by this random number generator
         */
        NormalRNG(T mean, T variance) :
            storedval(0.0), rng(new UniformRNG<T>(-1, 1)), mean(mean), variance(variance) {}
        /**
         * @brief Construct a new random number generator with normally distributed samples.
         * 
         * This implementation uses a standard random number generator and will produce
         * values with zero mean and unit variance.
         */
        NormalRNG() :
            storedval(0.0), rng(new UniformRNG<T>(-1, 1)), mean(0.0), variance(1.0) {}
        ~NormalRNG()    {delete rng;}
        T rand() const
        {
            //Box-Muller transform, as presented in "Numerical Recipes, Third edition", p.365...
            T v1, v2, rsq, fac;
            if (storedval == 0.0)
            {
                do
                {
                    v1 = rng->rand();
                    v2 = rng->rand();
                    rsq = v1*v1 + v2*v2;
                } while ((rsq >= 1.0) || (rsq == 0.0));
                fac = sqrt(-2.0*log(rsq)/rsq);
                storedval = v1 * fac;
                return mean + variance * v2 * fac;
            }
            else
            {
                fac = storedval;
                storedval = 0.0;
                return mean + variance * fac;
            }
        }
        
        /**
         * @brief Returns the mean of the RNG.
         * @return the mean of the RNG.
         */
        T getMean() const       {return mean;}
        /**
         * @brief Returns the variance of the RNG.
         * @return the variance of the RNG.
         */
        T getVariance() const   {return variance;}
    };
}

#endif  //RANDOMNUMBERS_HPP

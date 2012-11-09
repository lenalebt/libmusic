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
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename T>
    class StandardRNG
    {
    public:
        virtual T rand()=0;
    };
    
    /**
     * @brief A random number generator which produces uniformly diistributed random numbers.
     * 
     * @ingroup tools
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename T>
    class UniformRNG : public StandardRNG<T>
    {
    private:
        
    protected:
        T a;
        T b;
        T diff;
    public:
        UniformRNG() :
            a(0.0), b(1.0)  {diff = b-a;}
        UniformRNG(T a, T b) :
            a(a), b(b)      {diff = b-a;}
        T rand()
        {
            return a + (diff)*(double(std::rand()) / RAND_MAX);
        }
        
        T getA() const   {return a;}
        T getB() const   {return b;}
    };
    
    /**
     * @brief A random number generator which produces normally distributed numbers.
     * 
     * @ingroup tools
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename T>
    class NormalRNG : public StandardRNG<T>
    {
    private:
        double storedval;
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
         *      The uniform random number generator is assumed to produce valus in [0,1[.
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
        T rand()
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
        
        T getMean() const       {return mean;}
        T getVariance() const   {return variance;}
    };
}

#endif  //RANDOMNUMBERS_HPP

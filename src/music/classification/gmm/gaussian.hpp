#ifndef GAUSSIAN_HPP
#define GAUSSIAN_HPP

#include <Eigen/Dense>
#include "randomnumbers.hpp"
#include "json/json.h"
#include "fft.hpp"

namespace music
{
    /**
     * @brief This class represents a multivariate gaussian distribution.
     * 
     * A multivariate gaussian distribution is a probability density function.
     * It can be calculated via
     * \f[
     *      f_\mathbf{x}(x_1,\ldots,x_k)\, =
     *      \frac{1}{(2\pi)^{k/2}|\mathbf\Sigma|^{1/2}}
     *      \exp\left(-\frac{1}{2}({\mathbf x}-{\mathbf\mu})^T{\mathbf\Sigma}^{-1}({\mathbf x}-{\mathbf\mu})
     *      \right)
     * \f]
     * with \f$\mathbf\Sigma\f$ being the covariance matrix and \f$\mathbf\mu\f$ being the mean of
     * the distribution.
     * 
     * Here, an additional weight is applied to the density function, as it will
     * be used together with other densities to form a combined density function.
     * The weight is necessary as all probability density functions need to
     * fulfill the equation
     * \f[
     *      \int f_\mathbf{x}(x)dx = 1\quad.
     * \f]
     * 
     * Since some algorithms perform better when diagonal covariance matricies
     * are used, there are two possible derivatives of this class: One using full
     * covariance matricies, and the other restricting the covariance matrix to
     * its diagonal form.
     * 
     * @ingroup classification
     * @see GaussianDiagCov
     * @see GaussianFullCov
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=kiss_fft_scalar>
    class Gaussian : public StandardRNG<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >
    {
    private:
        
    protected:
        double weight;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean;
        double preFactor;
        double preFactorWithoutWeights;
        NormalRNG<ScalarType>* rng;
        bool externalRNG;
        Eigen::LLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > llt;
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>* pseudoInverse;
        
        /**
         * @brief Calculates the prefactor of the gaussian, which is used in
         *      every calculation of the pdf.
         * 
         * This differs from full covariance matrix to diagonal covariance matrix,
         * so every subclass needs to calculate this value on its own.
         */
        virtual void calculatePrefactor()=0;
        
    public:
        /**
         * @brief Creates a new gaussian with dimension <code>dimension</code>.
         * 
         * @param dimension The dimension of the gaussian.
         * @param rng The random number generator that will be used for random
         *      numbers. If you give one to the constructor, you will be responsible
         *      for deleting it. It you don't supply one, the object will create an
         *      own one. Can be used to share one RNG per thread.
         */
        Gaussian(unsigned int dimension, NormalRNG<ScalarType>* rng = NULL);
        /**
         * @brief Creates a new gaussian with weight <code>weight</code>
         *      and mean <code>mean</code>.
         * 
         * @param weight The weight of the gaussian. Should be from [0,1].
         * @param rng The random number generator that will be used for random
         *      numbers. If you give one to the constructor, you will be responsible
         *      for deleting it. It you don't supply one, the object will create an
         *      own one. Can be used to share one RNG per thread.
         */
        Gaussian(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng = NULL);
        Gaussian(const Gaussian<ScalarType>& other);
        virtual ~Gaussian();
        /**
         * @brief Calculate the value of the gaussian distribution at the given position.
         * @return The value of the pdf at the given position.
         */
        virtual double calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)=0;

        /**
         * @brief Calculate the value of the gaussian distribution at the given position without applying the weight.
         * @return The value of the pdf at the given position without the weight applied.
         */
        virtual double calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)=0;
        /**
         * @brief Calculate the value of the gaussian distribution with mean
         *      at position zero.
         * 
         * This function might be better if you know that the mean is zero, or if you
         * previously calculated the value-mean difference because you needed it elsewhere.
         * 
         * @return The value of the pdf at the given position if the mean was zero.
         */
        virtual double calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)=0;
        
        /**
         * @brief Get the weight factor of this gaussian.
         * @return The weight factor of this gaussian distribution.
         */
        double getWeight() const                    {return weight;}
        /**
         * @brief Set the weight factor of this gaussian.
         * @param weight The weight factor of this gaussian distribution.
         */
        void setWeight(double weight)               {this->weight = weight; calculatePrefactor();}
        
        /**
         * @brief Get the mean of this gaussian.
         * @return The mean of this gaussian distribution.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& getMean() const      {return mean;}
        /**
         * @brief Set the mean of this gaussian.
         * @param mean The mean of this gaussian distribution.
         */
        void setMean(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean)   {this->mean = mean; calculatePrefactor();}
        
        /**
         * @brief Get the covariance matrix of this gaussian.
         * @remarks This function will always give full covariance matricies,
         *      even if the matrix is not represented as such internally.
         * @return The covariance matrix of this gaussian distribution.
         */
        virtual Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> getCovarianceMatrix() const=0;
        /**
         * @brief Set the covariance matrix of this gaussian.
         * @remarks This function will always take full covariance matricies,
         *      even if the matrix is not represented as such internally.
         *      If this is the case, only the diagonal elements will be used!
         * @param matrix The covariance matrix of this gaussian distribution.
         */
        virtual void setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& matrix)=0;
        
        /**
         * @brief Generate a random vector that follows the distribution.
         * 
         * This function 
         * 
         * @return A random vector following this distribution.
         */
        virtual Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand() const=0;
        
        /**
         * @brief Returns a copy of the object.
         * 
         * Needed to copy the object if you only have a pointer to the base class, but want a copy of the derived one.
         * 
         * @return A pointer to a copy of the object.
         */
        virtual Gaussian<ScalarType>* clone()=0;
        
        /**
         * @brief Calculates the Mahalanobis distance of the given vectors
         * 
         * @param vector1 The first vector
         * @param vector2 The second vector. If omitted, the mean of the
         *      gaussian will be taken, so one is able to calculate
         *      "the distance from the gaussian".
         * 
         * @return the Mahalanobis distance of the given vectors
         */
        double calculateDistance(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector1, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector2);
        double calculateDistance(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector1);
        
        /**
         * @brief Loads a model directly from a JSON string.
         * 
         * The structure of the JSON string here is as follows:
         * @code
         * [
         *      "weight"     : double
         *      "mean"       : [double, double, ..._n]
         *      "covariance" : [double, double, ..._m]
         * ]
         * @endcode
         * The following equation
         * holds:
         * \f[
         *     m\in\left\{ n, \frac{n^2+n}{2n^2} \right\}
         * \f]
         * Typically, \f$k<20\f$ and \f$n<16\f$, but these are soft boundaries.
         * There are no hard boundaries.
         * 
         * If
         * \f$m=n\f$, then the elements of the covariance array are the diagonal
         * elements of the covariance matrix. If \f$m=\frac{n^2+n}{2n^2}\f$,
         * then the elements are the elements of the lower triangular of
         * the covariance matrix. The other elements can be generated through
         * mirroring the lower triangular on the diagonal. The order of the
         * elements is as follows:
         * \f[
         *      \left(
         *      \begin{array}{ccccc}
         *          a_0 & a_1 & a_3 & a_6 & \cdots\\
         *          a_1 & a_2 & a_4 & a_7 & \cdots\\
         *          a_3 & a_4 & a_5 & a_8 & \cdots\\
         *          a_6 & a_7 & a_8 & a_9 & \cdots\\
         *          \vdots & \vdots & \vdots & \vdots & \ddots\\
         *      \end{array}
         *      \right)
         * \f]
         * 
         * Typically, the JSON is condensed to save memory and speed the process up a bit.
         * 
         * @param jsonString A JSON string describing a gaussian.
         * @bug If m!=n, m=(n^2+n)/(2n^2) will be assumed. This is a security problem,
         *      as one might feed more elements to the program. In this case,
         *      program memory might be overwritten, or a crash occurs.
         *      The algorithm should be changed to address
         *      this problem.
         */
        static Gaussian<ScalarType>* loadFromJSONString(const std::string& jsonString);
        std::string toJSONString(bool styledWriter=false) const;
        static Gaussian<ScalarType>* loadFromJsonValue(Json::Value& jsonValue);
        
        virtual bool isFullCov()=0;
    };
    
    /**
     * @brief Represents a gaussian distribution with full covariance matricies.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=kiss_fft_scalar>
    class GaussianFullCov : public Gaussian<ScalarType>
    {
    private:
        
    protected:
        //weird things with templates and derived classes...
        using Gaussian<ScalarType>::mean;
        using Gaussian<ScalarType>::preFactor;
        using Gaussian<ScalarType>::preFactorWithoutWeights;
        using Gaussian<ScalarType>::rng;
        using Gaussian<ScalarType>::weight;
        using Gaussian<ScalarType>::llt;
        using Gaussian<ScalarType>::pseudoInverse;
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> fullCov;
        Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt;
        
        void calculatePrefactor();
    public:
        /**
         * @brief Creates a new gaussian with dimension <code>dimension</code>.
         * 
         * @param dimension The dimension of the gaussian.
         * @param rng The random number generator that will be used for random
         *      numbers. If you give one to the constructor, you will be responsible
         *      for deleting it. It you don't supply one, the object will create an
         *      own one. Can be used to share one RNG per thread.
         */
        GaussianFullCov(unsigned int dimension, NormalRNG<ScalarType>* rng = NULL);
        /**
         * @brief Creates a new gaussian with weight <code>weight</code>
         *      and mean <code>mean</code>.
         * 
         * @param weight The weight of the gaussian. Should be from [0,1].
         * @param rng The random number generator that will be used for random
         *      numbers. If you give one to the constructor, you will be responsible
         *      for deleting it. It you don't supply one, the object will create an
         *      own one. Can be used to share one RNG per thread.
         */
        GaussianFullCov(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng = NULL);
        GaussianFullCov(const GaussianFullCov<ScalarType>& other);
        double calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        double calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        double calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        void setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& matrix);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> getCovarianceMatrix() const   {return fullCov;}
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand() const;
        Gaussian<ScalarType>* clone();
        
        bool isFullCov() {return true;}
    };
    
    /**
     * @brief Represents a gaussian distribution with diagonal covariance matricies.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=kiss_fft_scalar>
    class GaussianDiagCov : public Gaussian<ScalarType>
    {
    private:
        
    protected:
        //weird things with templates and derived classes...
        using Gaussian<ScalarType>::mean;
        using Gaussian<ScalarType>::preFactor;
        using Gaussian<ScalarType>::preFactorWithoutWeights;
        using Gaussian<ScalarType>::rng;
        using Gaussian<ScalarType>::weight;
        using Gaussian<ScalarType>::llt;
        using Gaussian<ScalarType>::pseudoInverse;
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diagCov;
        Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt;
        
        void calculatePrefactor();
    public:
        /**
         * @brief Creates a new gaussian with dimension <code>dimension</code>.
         * 
         * @param dimension The dimension of the gaussian.
         * @param rng The random number generator that will be used for random
         *      numbers. If you give one to the constructor, you will be responsible
         *      for deleting it. It you don't supply one, the object will create an
         *      own one. Can be used to share one RNG per thread.
         */
        GaussianDiagCov(unsigned int dimension, NormalRNG<ScalarType>* rng = NULL);
        /**
         * @brief Creates a new gaussian with weight <code>weight</code>
         *      and mean <code>mean</code>.
         * 
         * @param weight The weight of the gaussian. Should be from [0,1].
         * @param rng The random number generator that will be used for random
         *      numbers. If you give one to the constructor, you will be responsible
         *      for deleting it. It you don't supply one, the object will create an
         *      own one. Can be used to share one RNG per thread.
         */
        GaussianDiagCov(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng = NULL);
        GaussianDiagCov(const GaussianDiagCov<ScalarType>& other);
        double calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        double calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        double calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> getCovarianceMatrix() const  {return diagCov.asDiagonal();}
        void setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& matrix);
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand() const;
        Gaussian<ScalarType>* clone();
        
        bool isFullCov() {return false;}
    };
}

#endif  //GAUSSIAN_HPP

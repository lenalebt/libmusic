#ifndef GMM_HPP
#define GMM_HPP

#include <Eigen/Dense>
#include <vector>
#include <Eigen/Cholesky>

#include "randomnumbers.hpp"
#include <iostream>
#include "json/json.h"
#include "debug.hpp"
#include "matrixhelper.hpp"

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
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=double>
    class Gaussian : public StandardRNG<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >
    {
    private:
        
    protected:
        ScalarType weight;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean;
        ScalarType preFactor;
        ScalarType preFactorWithoutWeights;
        NormalRNG<ScalarType>* rng;
        
        /**
         * @brief Calculates the prefactor of the gaussian, which is used in
         *      every calculation of the pdf.
         * 
         * This differs from full covariance matrix to diagonal covariance matrix,
         * so every subclass needs to calculate this value on its own.
         */
        virtual void calculatePrefactor()=0;
    public:
        Gaussian(unsigned int dimension);
        Gaussian(ScalarType weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean);
        Gaussian(const Gaussian<ScalarType>& other);
        /**
         * @brief Calculate the value of the gaussian distribution at the given position.
         * @return The value of the pdf at the given position.
         */
        virtual ScalarType calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)=0;
        /**
         * @brief Calculate the value of the gaussian distribution at the given position without applying the weight.
         * @return The value of the pdf at the given position without the weight applied.
         */
        virtual ScalarType calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)=0;
        /**
         * @brief Calculate the value of the gaussian distribution with mean
         *      at position zero.
         * 
         * This function might be better if you know that the mean is zero, or if you
         * previously calculated the value-mean difference because you needed it elsewhere.
         * 
         * @return The value of the pdf at the given position if the mean was zero.
         */
        virtual ScalarType calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)=0;
        
        /**
         * @brief Get the weight factor of this gaussian.
         * @return The weight factor of this gaussian distribution.
         */
        ScalarType getWeight() const                    {return weight;}
        /**
         * @brief Set the weight factor of this gaussian.
         * @param weight The weight factor of this gaussian distribution.
         */
        void setWeight(ScalarType weight)               {this->weight = weight; calculatePrefactor();}
        
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
        virtual Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> getCovarianceMatrix()=0;
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
        virtual Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand()=0;
    };
    
    /**
     * @brief Represents a gaussian distribution with full covariance matricies.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=double>
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
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> fullCov;
        Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt;
        
        void calculatePrefactor();
    public:
        GaussianFullCov(unsigned int dimension);
        GaussianFullCov(ScalarType weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean);
        GaussianFullCov(const GaussianFullCov<ScalarType>& other);
        ScalarType calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        ScalarType calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        ScalarType calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        void setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& matrix);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> getCovarianceMatrix()   {return fullCov;}
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand();
    };
    
    /**
     * @brief Represents a gaussian distribution with diagonal covariance matricies.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=double>
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
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diagCov;
        Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt;
        
        void calculatePrefactor();
    public:
        GaussianDiagCov(unsigned int dimension);
        GaussianDiagCov(ScalarType weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean);
        GaussianDiagCov(const GaussianDiagCov<ScalarType>& other);
        ScalarType calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        ScalarType calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        ScalarType calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> getCovarianceMatrix()   {return diagCov.asDiagonal();}
        void setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& matrix);
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand();
    };
    
    /**
     * @brief This class is able to generate a gaussian mixture model for data.
     * 
     * If you have a fairly large dataset (some 1,000-10,000 data points),
     * this class helps you find a model which represents the data. It uses
     * a mixture of normal distributions to model the data. If you know the
     * structure of your data to be different, it might be better to use a
     * different approach.
     * 
     * To use this class, you need to know all your data at once and call the trainGMM()
     * function on the data. It will use a suitable unsupervized learning algorithm
     * to build the model (usually the EM algorithm).
     * 
     * @ingroup classification
     * @see GaussianMixtureModelDiagCov
     * @see GaussianMixtureModelFullCov
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=double>
    class GaussianMixtureModel : public StandardRNG<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >
    {
    private:
        
    protected:
        std::vector<Gaussian<ScalarType>*> gaussians;
        UniformRNG<ScalarType> uniRNG;
        ScalarType normalizationFactor;
        
        /**
         * @brief Load the JSON from a JSON representation in memory.
         * @see loadFromJSONString()
         */
        void loadFromJsonValue(Json::Value& jsonValue);
        
        /**
         * @brief Starts the expectation-maximization algorithm (EM algorithm) for
         *      gaussian mixture models (GMM) on the data.
         * 
         * @param init The initial guesses for the centers of gravity of the normal
         *      distributions
         * @param data The data that should be analyzed
         * @param gaussianCount The count of gaussian distributions that will be used to model the data
         * @param maxIterations The maximum number of iterations of the algorithm. Usually, it converges much faster.
         * 
         * @remarks Keep in mind that the gaussians have a field called "weight",
         *      which tells you the weight of the gaussian in the model.
         * 
         * @return A list of the gaussian distributions that build the model.
         */
        virtual std::vector<Gaussian<ScalarType>* > emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount = 10, unsigned int maxIterations=50)=0;
    public:
        /**
         * @brief Creates a new empty Gaussian Mixture Model.
         */
        GaussianMixtureModel();
        /**
         * @brief Train this GMM to model the data given.
         * 
         * @param data The data that should be modeled
         * @param gaussianCount The count of gaussian distributions that will be used to model the data
         * 
         */
        void trainGMM(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount=10);
        /**
         * @brief Compare this gaussian mixture model with another one.
         * 
         * @remarks It is not clear which distance measure will be used for
         *      this task. It is not clear if the measure will be a number
         *      in [0,1] where 1 means "fits good" to 0 means "does not fit",
         *      or if it should be a distance measure where 0 means "they are equal or very close"
         *      to \f$\infty\f$ which means "imagine anything that does not fit, here it is".
         * @remarks There are some approaches: Earth Movers Distance, or by sampling.
         * 
         * @todo Implement!
         * 
         * @return A distance measure of the two gaussians.
         */
        ScalarType compareTo(const GaussianMixtureModel<ScalarType>& other);
        
        /**
         * @brief Returns the list of gaussian distributions of this model.
         * @return A list of gaussian distributions.
         */
        std::vector<Gaussian<ScalarType>*> getGaussians() const {return gaussians;}
        
        /**
         * @brief Draw a random vector from the probability density that is
         *      represented by this GMM.
         * @return A random vector drawn from this probability density.
         */
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand();
        
        /**
         * @brief Computes the value of the probability density function
         *      represented by this GMM at the given position.
         * @return The value of the probability density at the given position
         */
        ScalarType calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& pos) const;
        
        /**
         * @brief Create a JSON string that represents this model.
         * @param styledWriter If a styled writer should be used, or not. With a styled writer,
         *      the format is easier to read for humans, but takes up more
         *      memory. If you don't supply this parameter, a condensed
         *      JSON will be produced.
         * @return A JSON string representing this model.
         * @see loadFromJSONString() for the structure of the JSON string.
         */
        std::string toJSONString(bool styledWriter=false) const;
        /**
         * @brief Loads a model directly from a JSON string.
         * 
         * The structure of the JSON string here is as follows:
         * @code
         * [
         *   {
         *      "weight"     : double
         *      "mean"       : [double, double, ..._n]
         *      "covariance" : [double, double, ..._m]
         *   }
         *   , ..._k
         * ]
         * @endcode
         * Where ..._i means the number of repetitions. The following equation
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
         * @param jsonString A JSON string describing a gaussian mixture model.
         * @bug If m!=n, m=(n^2+n)/(2n^2) will be assumed. This is a security problem,
         *      as one might feed more elements to the program. In this case,
         *      program memory might be overwritten, or a crash occurs.
         *      The algorithm should be changed to address
         *      this problem.
         */
        void loadFromJSONString(const std::string& jsonString);
        
        /**
         * @brief Creates a new GMM, which is a mixture of the two GMMs that are added.
         * 
         * @remarks The pointer returned will not be deleted by this class,
         *      you need to take care of it on your own. If you call
         *      <code>gmm1 + gmm2</code> without deleting the resulting pointer,
         *      you will get memory leaks.
         * 
         * @return A pointer to the new GMM.
         */
        GaussianMixtureModel<ScalarType>* operator+(const GaussianMixtureModel<ScalarType>& other);
        
        template <typename S>
        friend std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<S>& model);
        template <typename S>
        friend std::istream& operator>>(std::istream& is, GaussianMixtureModel<S>& model);
    };
    /*template <typename ScalarType> class GaussianMixtureModel<ScalarType, GaussianFullCov<ScalarType> >;
    template <typename ScalarType> class GaussianMixtureModel<ScalarType, GaussianDiagCov<ScalarType> >;*/
    
    /**
     * @brief A GaussianMixtureModel which uses full covariance matricies.
     * 
     * This class performs slower calculations than GaussianMixtureModelDiagCov,
     * but the results are more accurate than they are with the other class.
     * 
     * @see GaussianMixtureModel
     * @see GaussianMixtureModelDiagCov
     * @see GaussianFullCov
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-09-10
     */
    template <typename ScalarType>
    class GaussianMixtureModelFullCov : public GaussianMixtureModel<ScalarType>
    {
        protected:
            using GaussianMixtureModel<ScalarType>::gaussians;
            using GaussianMixtureModel<ScalarType>::uniRNG;
            using GaussianMixtureModel<ScalarType>::normalizationFactor;
            
            std::vector<Gaussian<ScalarType>* > emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount = 10, unsigned int maxIterations=50);
            
    };
    
    /**
     * @brief A GaussianMixtureModel which uses diagonal covariance matricies.
     * 
     * This class performs faster calculations than GaussianMixtureModelFullCov,
     * but the results are not as accurate as they are with the other class.
     * 
     * @see GaussianMixtureModel
     * @see GaussianMixtureModelFullCov
     * @see GaussianDiagCov
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-09-10
     */
    template <typename ScalarType>
    class GaussianMixtureModelDiagCov : public GaussianMixtureModel<ScalarType>
    {
        protected:
            using GaussianMixtureModel<ScalarType>::gaussians;
            using GaussianMixtureModel<ScalarType>::uniRNG;
            using GaussianMixtureModel<ScalarType>::normalizationFactor;
            
            std::vector<Gaussian<ScalarType>* > emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount = 10, unsigned int maxIterations=50);
    };
    
    template <typename ScalarType> std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<ScalarType>& model);
    template <typename ScalarType> std::istream& operator>>(std::istream& is, GaussianMixtureModel<ScalarType>& model);
    
    //NEW IMPLEMENTATION
    template <typename ScalarType>
    class GMM2
    {
    private:
        
    public:
        void trainGMMDiag(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianount = 10, unsigned int maxIterations=50);
        void trainGMMFull(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianount = 10, unsigned int maxIterations=50);
    };
}

#endif //GMM_HPP

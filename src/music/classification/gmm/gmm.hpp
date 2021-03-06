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
#include "fft.hpp"

#include "gaussian.hpp"

namespace music
{
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
    template <typename ScalarType=kiss_fft_scalar>
    class GaussianMixtureModel : public StandardRNG<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >
    {
    private:
        
    protected:
        std::vector<Gaussian<ScalarType>*> gaussians;
        UniformRNG<ScalarType> uniRNG;
        static NormalRNG<ScalarType>* normalRNG;
        ScalarType normalizationFactor;
        
        double aic, aicc, bic, loglike;
        
        /**
         * @brief Calculates the AIC, AICc, BIC and stores them internally.
         * 
         * @param k The number of free parameters in the model
         * @param n The number of data points used to calculate the model
         * @param loglike The log-likelihood of the model
         */
        void calculateInformationCriteria(int k, int n, double loglike);
        
        /**
         * @brief Starts the expectation-maximization algorithm (EM algorithm) for
         *      gaussian mixture models (GMM) on the data.
         * 
         * @param init The initial guesses for the centers of gravity and covariance matricies
         *      of the normal distributions. Setting the diagonal elements of the
         *      covariance matrix to approximately 10000 is a good initial guess.
         * @param data The data that should be analyzed
         * @param gaussianCount The count of gaussian distributions that will be used to model the data
         * @param maxIterations The maximum number of iterations of the algorithm. Usually, it converges much faster.
         * 
         * @remarks Keep in mind that the gaussians have a field called "weight",
         *      which tells you the weight of the gaussian in the model.
         * 
         * @return A list of the gaussian distributions that build the model.
         */
        virtual std::vector<Gaussian<ScalarType>* > emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount = 10, unsigned int maxIterations=50, double initVariance = 100.0, double minVariance = 0.1)=0;
    public:
        /**
         * @brief Creates a new empty Gaussian Mixture Model.
         */
        GaussianMixtureModel();
        
        /**
         * @brief Creates a copy of the Gaussian Mixture Model.
         */
        GaussianMixtureModel(const GaussianMixtureModel<ScalarType>& other);
        
        virtual ~GaussianMixtureModel();
        
        /**
         * @brief Train this GMM to model the data given.
         * 
         * @param data The data that should be modeled. You need to give at
         *      least as many data points to the algorithm as the dimension of the
         *      data is, otherwise the algorithm fails (built-in assertion).
         *      Ideally, you would give around <code>10^dimension</code>
         *      data points to the algorithm. As you will normally not have that many data
         *      points, the rule of thumb is "the more data, the better the model!".
         * @param gaussianCount The count of gaussian distributions that will be used to model the data
         * @param initVariance The initial variance (diagonal) of the covariance matricies.
         * @param minVariance The minimum variance (diagonal) of the covariance matricies. If the variance gets smaller
         *      than this value, it will be set to this value.
         * 
         */
        void trainGMM(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount=10, double initVariance = 100.0, double minVariance = 0.1);
        
        /**
         * @brief Returns the Akaike Information Criterion of the model.
         * 
         * @see http://en.wikipedia.org/wiki/Akaike_information_criterion
         * @attention This value only is available if the model has been trained. If it has been loaded
         *      via JSON, it might not be available and thus be zero. The reason is that it is mostly
         *      useful for model selection, and this has to be done right after training.
         * 
         * @return the AIC of the model
         */
        double getModelAIC()            {return aic;}
        /**
         * @brief Returns the corrected Akaike Information Criterion for small sample sizes of the model.
         * 
         * @see http://en.wikipedia.org/wiki/Akaike_information_criterion
         * @attention This value only is available if the model has been trained. If it has been loaded
         *      via JSON, it might not be available and thus be zero. The reason is that it is mostly
         *      useful for model selection, and this has to be done right after training.
         * 
         * @return the AICc of the model
         */
        double getModelAICc()           {return aicc;}
        /**
         * @brief Returns the Bayesian Information Criterion of the model.
         * 
         * @see http://en.wikipedia.org/wiki/Bayesian_information_criterion
         * @attention This value only is available if the model has been trained. If it has been loaded
         *      via JSON, it might not be available and thus be zero. The reason is that it is mostly
         *      useful for model selection, and this has to be done right after training.
         * 
         * @return the BIC of the model
         */
        double getModelBIC()            {return bic;}
        /**
         * @brief Returns the log-likelihood of the model as calculated in the EM-algorithm.
         * 
         * @attention This value only is available if the model has been trained. If it has been loaded
         *      via JSON, it might not be available and thus be zero. The reason is that it is mostly
         *      useful for model selection, and this has to be done right after training.
         * 
         * @return the log-likelihood of the model
         */
        double getModelLogLikelihood()  {return loglike;}
        
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
         * @return A distance measure of the two gaussians.
         */
        ScalarType compareTo(const GaussianMixtureModel<ScalarType>& other, int sampleCount=200);
        
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
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> rand() const;
        
        /**
         * @brief Computes the value of the probability density function
         *      represented by this GMM at the given position.
         * @return The value of the probability density at the given position
         */
        ScalarType calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& pos) const;
        ScalarType calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& pos) const;
        
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
        static GaussianMixtureModel<ScalarType>* loadFromJSONString(const std::string& jsonString);
        
        /**
         * @brief Load the JSON from a JSON representation in memory.
         * 
         * If you don't know about this function, you should probably use
         * loadFromJSONString().
         * 
         * @see loadFromJSONString()
         */
        static GaussianMixtureModel<ScalarType>* loadFromJsonValue(Json::Value& jsonValue);
        
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
        
        /**
         * @brief Returns a copy of the object.
         * 
         * Needed to copy the object if you only have a pointer to the base class, but want a copy of the derived one.
         * 
         * @return A pointer to a copy of the object.
         */
        virtual GaussianMixtureModel<ScalarType>* clone()=0;
        
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
    template <typename ScalarType=kiss_fft_scalar>
    class GaussianMixtureModelFullCov : public GaussianMixtureModel<ScalarType>
    {
        protected:
            using GaussianMixtureModel<ScalarType>::gaussians;
            using GaussianMixtureModel<ScalarType>::uniRNG;
            using GaussianMixtureModel<ScalarType>::normalRNG;
            using GaussianMixtureModel<ScalarType>::normalizationFactor;
            
            std::vector<Gaussian<ScalarType>* > emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount = 10, unsigned int maxIterations=50, double initVariance = 100.0, double minVariance = 0.1);
        public:
            GaussianMixtureModel<ScalarType>* clone();
            GaussianMixtureModelFullCov(const GaussianMixtureModelFullCov<ScalarType>& other);
            GaussianMixtureModelFullCov();
            ~GaussianMixtureModelFullCov() {}
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
    template <typename ScalarType=kiss_fft_scalar>
    class GaussianMixtureModelDiagCov : public GaussianMixtureModel<ScalarType>
    {
        protected:
            using GaussianMixtureModel<ScalarType>::gaussians;
            using GaussianMixtureModel<ScalarType>::uniRNG;
            using GaussianMixtureModel<ScalarType>::normalRNG;
            using GaussianMixtureModel<ScalarType>::normalizationFactor;
            
            std::vector<Gaussian<ScalarType>* > emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount = 10, unsigned int maxIterations=50, double initVariance = 100.0, double minVariance = 0.1);
        public:
            GaussianMixtureModel<ScalarType>* clone();
            GaussianMixtureModelDiagCov(const GaussianMixtureModelDiagCov<ScalarType>& other);
            GaussianMixtureModelDiagCov();
            ~GaussianMixtureModelDiagCov() {}
    };
    
    template <typename ScalarType> std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<ScalarType>& model);
    template <typename ScalarType> std::istream& operator>>(std::istream& is, GaussianMixtureModel<ScalarType>& model);
}

#endif //GMM_HPP

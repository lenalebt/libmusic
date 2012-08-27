#ifndef GMM_HPP
#define GMM_HPP

#include <Eigen/Dense>
#include <vector>
#include <Eigen/Cholesky>

#include "randomnumbers.hpp"

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
    class Gaussian : public StandardRNG<Eigen::VectorXd>
    {
    private:
        
    protected:
        double weight;
        Eigen::VectorXd mean;
        double preFactor;
        NormalRNG<double>* rng;
        
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
        Gaussian(double weight, Eigen::VectorXd mean);
        /**
         * @brief Calculate the value of the gaussian distribution at the given position.
         * @return The value of the pdf at the given position.
         */
        virtual double calculateValue(const Eigen::VectorXd& dataVector)=0;
        /**
         * @brief Calculate the value of the gaussian distribution with mean
         *      at position zero.
         * 
         * This function might be better if you know that the mean is zero, or if you
         * previously calculated the value-mean difference because you needed it elsewhere.
         * 
         * @return The value of the pdf at the given position if the mean was zero.
         */
        virtual double calculateNoMeanValue(const Eigen::VectorXd& dataVector)=0;
        
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
        const Eigen::VectorXd& getMean() const      {return mean;}
        /**
         * @brief Set the mean of this gaussian.
         * @param mean The mean of this gaussian distribution.
         */
        void setMean(const Eigen::VectorXd& mean)   {this->mean = mean; calculatePrefactor();}
        
        /**
         * @brief Get the covariance matrix of this gaussian.
         * @remarks This function will always give full covariance matricies,
         *      even if the matrix is not represented as such internally.
         * @return The covariance matrix of this gaussian distribution.
         */
        virtual Eigen::MatrixXd getCovarianceMatrix()=0;
        /**
         * @brief Set the covariance matrix of this gaussian.
         * @remarks This function will always take full covariance matricies,
         *      even if the matrix is not represented as such internally.
         *      If this is the case, only the diagonal elements will be used!
         * @param matrix The covariance matrix of this gaussian distribution.
         */
        virtual void setCovarianceMatrix(const Eigen::MatrixXd& matrix)=0;
        
        /**
         * @brief Generate a random vector that follows the distribution.
         * 
         * This function 
         * 
         * @return A random vector following this distribution.
         */
        virtual Eigen::VectorXd rand()=0;
    };
    
    /**
     * @brief Represents a gaussian distribution with full covariance matricies.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    class GaussianFullCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::MatrixXd fullCov;
        Eigen::LDLT<Eigen::MatrixXd> ldlt;
        
        void calculatePrefactor();
    public:
        GaussianFullCov(unsigned int dimension);
        double calculateValue(const Eigen::VectorXd& dataVector);
        double calculateNoMeanValue(const Eigen::VectorXd& dataVector);
        void setCovarianceMatrix(const Eigen::MatrixXd& matrix);
        Eigen::MatrixXd getCovarianceMatrix()   {return fullCov;}
        
        Eigen::VectorXd rand();
    };
    
    /**
     * @brief Represents a gaussian distribution with diagonal covariance matricies.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    class GaussianDiagCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::VectorXd diagCov;
        Eigen::VectorXd diagCovInverse;
        double diagCovDeterminant;
        
        void calculatePrefactor();
    public:
        double calculateValue(const Eigen::VectorXd& dataVector);
        double calculateNoMeanValue(const Eigen::VectorXd& dataVector);
        Eigen::MatrixXd getCovarianceMatrix()   {return diagCov.asDiagonal();}
        void setCovarianceMatrix(const Eigen::MatrixXd& matrix) {this->diagCov = matrix.diagonal();}
        
        Eigen::VectorXd rand();
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
     * to build the model.
     * 
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    class GaussianMixtureModel : public StandardRNG<Eigen::VectorXd>
    {
    private:
        
    protected:
        std::vector<Gaussian*> gaussians;
        UniformRNG<double> uniRNG;
        
        //give back gaussian with weight.
        /**
         * @brief Starts the expectation-maximization algorithm (EM algorithm) on the data.
         * 
         * 
         * 
         * @return A list of the gaussian distributions that build the model.
         */
        std::vector<Gaussian* > emAlg(std::vector<Gaussian*> init, std::vector<Eigen::VectorXd> data, int gaussianCount = 10, unsigned int maxIterations=50);
    public:
        GaussianMixtureModel();
        //use EM algorithm to train the model
        void trainGMM(std::vector<Eigen::VectorXd> data, int gaussianCount=10);
        //compare models (with Earth Movers Distance, or by sampling)
        double compareTo(const GaussianMixtureModel& other);
        
        std::vector<Gaussian*> getGaussians() const {return gaussians;}
        
        Eigen::VectorXd rand();
    };
}

#endif //GMM_HPP

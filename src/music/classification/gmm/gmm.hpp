#ifndef GMM_HPP
#define GMM_HPP

#include <Eigen/Dense>
#include <vector>
#include <Eigen/Cholesky>

namespace music
{
    class Gaussian
    {
    private:
        
    protected:
        double weight;
        Eigen::VectorXd mean;
    public:
        virtual double calculateValue(const Eigen::VectorXd& dataVector)=0;
        //calculate gaussian without subtraction of mean (might be done outside of the function!)
        virtual double calculateNoMeanValue(const Eigen::VectorXd& dataVector)=0;
        double getWeight() const                    {return weight;}
        const Eigen::VectorXd& getMean() const      {return mean;}
        void setMean(const Eigen::VectorXd& mean)   {this->mean = mean;}
        virtual Eigen::MatrixXd getCovarianceMatrix()=0;
        virtual void setCovarianceMatrix(const Eigen::MatrixXd& matrix)=0;
        
        //create random value distributed like this gaussian (pseudorandom, not very good)
        virtual Eigen::VectorXd rand()=0;
    };

    class GaussianFullCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::MatrixXd fullCov;
        Eigen::LDLT<Eigen::MatrixXd> ldlt;
        double preFactor;
    public:
        GaussianFullCov() : fullCov(), ldlt(), preFactor() {}
        double calculateValue(const Eigen::VectorXd& dataVector);
        double calculateNoMeanValue(const Eigen::VectorXd& dataVector);
        void setCovarianceMatrix(const Eigen::MatrixXd& matrix);
        Eigen::MatrixXd getCovarianceMatrix()   {return fullCov;}
        
        Eigen::VectorXd rand();
    };

    class GaussianDiagCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::VectorXd diagCov;
        Eigen::VectorXd diagCovInverse;
        double diagCovDeterminant;
        double preFactor;
    public:
        double calculateValue(const Eigen::VectorXd& dataVector);
        double calculateNoMeanValue(const Eigen::VectorXd& dataVector);
        Eigen::MatrixXd getCovarianceMatrix()   {return diagCov.asDiagonal();}
        void setCovarianceMatrix(const Eigen::MatrixXd& matrix) {this->diagCov = matrix.diagonal();}
        
        Eigen::VectorXd rand();
    };

    class GaussianMixtureModel
    {
    private:
        
    protected:
        std::vector<Gaussian*> gaussians;
        Eigen::VectorXd weights;
        
        //give back gaussian with weight.
        std::vector<std::pair<Gaussian*, double> > emAlg(std::vector<Gaussian*> init, std::vector<Eigen::VectorXd> data, int gaussianCount = 10,unsigned int maxIterations=50);
    public:
        //use EM algorithm to train the model
        void trainGMM(std::vector<Eigen::VectorXd> data, int gaussianCount=10);
        //compare models (with Earth Movers Distance, or by sampling)
        double compareTo(const GaussianMixtureModel& other);
        
        const Eigen::VectorXd& getWeights() const   {return weights;}
        std::vector<Gaussian*> getGaussians() const {return gaussians;}
    };
}

#endif //GMM_HPP

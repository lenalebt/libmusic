#ifndef GMM_HPP
#define GMM_HPP

#include <Eigen/Dense>
#include <vector>

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
        double getWeight()                          {return weight;}
        Eigen::VectorXd& getMean()                  {return mean;}
        void setMean(const Eigen::VectorXd& mean)   {this->mean = mean;}
        virtual Eigen::MatrixXd getCovarianceMatrix()=0;
        virtual void setCovarianceMatrix(const Eigen::MatrixXd& matrix)=0;
    };

    class GaussianFullCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::MatrixXd fullCov;
        Eigen::MatrixXd fullCovInverse;
        double fullCovDeterminant;
        double preFactor;
    public:
        double calculateValue(const Eigen::VectorXd& dataVector);
        void setCovarianceMatrix(const Eigen::MatrixXd& matrix);
        Eigen::MatrixXd getCovarianceMatrix()   {return fullCov;}
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
        Eigen::MatrixXd getCovarianceMatrix()   {return diagCov.asDiagonal();}
        void setCovarianceMatrix(const Eigen::MatrixXd& matrix) {this->diagCov = matrix.diagonal();}
    };

    class GaussianMixtureModel
    {
    private:
        
    protected:
        std::vector<Gaussian*> gaussians;
        
        //give back gaussian with weight.
        std::vector<std::pair<Gaussian*, double> > emAlg(std::vector<Gaussian*> init, std::vector<Eigen::VectorXd> data, int gaussianCount = 10,unsigned int maxIterations=50);
    public:
        //use EM algorithm to train the model
        void trainGMM(std::vector<Eigen::VectorXd> data, int gaussianCount=10);
        //compare models (with Earth Movers Distance, or by sampling)
        double compareTo(const GaussianMixtureModel& other);
    };
}

#endif //GMM_HPP

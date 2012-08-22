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
        double getWeight()          {return weight;}
        Eigen::VectorXd& getMean()  {return mean;}
        Eigen::MatrixXd getCovariance();
    };

    class GaussianFullCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::MatrixXd fullCov;
    public:
        double calculateValue(const Eigen::VectorXd& dataVector);
    };

    class GaussianDiagCov : public Gaussian
    {
    private:
        
    protected:
        Eigen::VectorXd diagCov;
    public:
        double calculateValue(const Eigen::VectorXd& dataVector);
    };

    class GaussianMixtureModel
    {
    private:
        
    protected:
        std::vector<Gaussian*> gaussians;
    public:
        //use EM algorithm to train the model
        void trainGMM(std::vector<Eigen::VectorXd> data, int gaussianCount);
        //compare models (with Earth Movers Distance, or by sampling)
        double compareTo(const GaussianMixtureModel& other);
    };
}

#endif //GMM_HPP

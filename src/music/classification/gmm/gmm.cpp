#include "gmm.hpp"

#include <Eigen/LU>
#include <assert.h>

namespace music
{
    double GaussianDiagCov::calculateValue(const Eigen::VectorXd& dataVector)
    {
        
    }
    
    
    void GaussianFullCov::setCovarianceMatrix(const Eigen::MatrixXd& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        //calculate inverse and determinant, as we need it many times
        this->fullCov = fullCov;
        this->fullCovInverse = fullCov.inverse();
        fullCovDeterminant = fullCov.determinant();
        
        preFactor = 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCovDeterminant));
    }
    double GaussianFullCov::calculateValue(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd dist = dataVector - mean;
        return preFactor * std::exp(-0.5 * (dist.transpose() * fullCovInverse * dist)(0));
    }
    
    std::vector<std::pair<Gaussian*, double> > GaussianMixtureModel::emAlg(std::vector<Gaussian*> init, std::vector<Eigen::VectorXd> data, unsigned int maxIterations)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        std::vector<Gaussian*> gaussians;
        int dimension = data[0].size();
        if (init.empty())
        {
            //init with random data points and identity matricies as covariance matrix
            for (int i=0; i<dimension; i++)
            {
                Gaussian* gaussian;
                
                gaussian->setMean(data[rand() % data.size()]);
                gaussian->setCovarianceMatrix(Eigen::MatrixXd::Identity(dimension, dimension));
                gaussians.push_back(gaussian);
            }
        }
        
        Eigen::VectorXd weights;
        
        
        unsigned int iteration = 0;
        bool converged = false;
        while ((iteration < maxIterations) && (!converged))
        {
            iteration++;
            
            //E-step BEGIN
            
            //E-step END
            
            //M-step BEGIN
            
            //M-step END
        }
        
        //TODO: return step
    }
}

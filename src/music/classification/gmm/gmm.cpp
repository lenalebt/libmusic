#include "gmm.hpp"

#include <Eigen/LU>
#include <assert.h>
#include "debug.hpp"

namespace music
{
    double GaussianDiagCov::calculateValue(const Eigen::VectorXd& dataVector)
    {
        //TODO
    }
    double GaussianDiagCov::calculateNoMeanValue(const Eigen::VectorXd& dataVector)
    {
        //TODO
    }
    Eigen::VectorXd GaussianDiagCov::rand()
    {
        //TODO
    }
    
    
    void GaussianFullCov::setCovarianceMatrix(const Eigen::MatrixXd& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        //calculate inverse and determinant, as we need it many times
        this->fullCov = fullCov;
        ldlt.compute(fullCov);
        
        preFactor = 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
    }
    double GaussianFullCov::calculateValue(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd dist = dataVector - mean;
        Eigen::VectorXd dist2 = dist;
        ldlt.solve(dist2);
        return preFactor * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    double GaussianFullCov::calculateNoMeanValue(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd vec2 = dataVector;
        ldlt.solve(vec2);
        return preFactor * std::exp(-0.5 * (dataVector.transpose() * vec2)(0));
    }
    Eigen::VectorXd GaussianFullCov::rand()
    {
        //TODO
    }
    
    void GaussianMixtureModel::trainGMM(std::vector<Eigen::VectorXd> data, int gaussianCount)
    {
        //TODO: precompute good starting vectors.
        std::vector<std::pair<Gaussian*, double> > gs = this->emAlg(std::vector<Gaussian*>(), data, gaussianCount);
        
        gaussians.clear();
        weights = Eigen::VectorXd(gaussianCount);
        for (int i=0; i<gaussianCount; i++)
        {
            gaussians.push_back(gs[i].first);
            weights[i] = gs[i].second;
        }
        
        DEBUG_OUT("gaussian 1 mean: " << gaussians[0]->getMean(), 0);
        DEBUG_OUT("gaussian 1 sigma: " << gaussians[0]->getCovarianceMatrix(), 0);
        DEBUG_OUT("gaussian 2 mean: " << gaussians[1]->getMean(), 0);
        DEBUG_OUT("gaussian 2 sigma: " << gaussians[1]->getCovarianceMatrix(), 0);
        DEBUG_OUT("gaussian 3 mean: " << gaussians[2]->getMean(), 0);
        DEBUG_OUT("gaussian 3 sigma: " << gaussians[2]->getCovarianceMatrix(), 0);
    }
    
    std::vector<std::pair<Gaussian*, double> > GaussianMixtureModel::emAlg(std::vector<Gaussian*> init, std::vector<Eigen::VectorXd> data, int gaussianCount, unsigned int maxIterations)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        std::vector<Gaussian*> gaussians;
        int dimension = data[0].size();
        unsigned int dataSize = data.size();
        if (init.empty())
        {
            DEBUG_OUT("no init vectors given. using random values...", 20);
            //init with random data points and identity matricies as covariance matrix
            for (int i=0; i<gaussianCount; i++)
            {
                DEBUG_OUT("adding gaussian distribution " << i << "...", 25);
                Gaussian* gaussian = new GaussianFullCov();
                
                gaussian->setMean(data[rand() % dataSize]);
                gaussian->setCovarianceMatrix(Eigen::MatrixXd::Identity(dimension, dimension));
                gaussians.push_back(gaussian);
            }
        }
        
        //set initial weights all equal to 1/gaussianCount
        Eigen::VectorXd weights = Eigen::VectorXd::Constant(gaussianCount, 1.0/double(gaussianCount));
        Eigen::VectorXd oldWeights = weights;
        Eigen::ArrayXXd p(dataSize, gaussianCount);
        
        
        unsigned int iteration = 0;
        bool converged = false;
        DEBUG_OUT("starting iteration loop...", 20);
        while ((iteration < maxIterations) && (!converged))
        {
            iteration++;
            oldWeights = weights;
            
            //E-step BEGIN
            DEBUG_OUT("E-step BEGIN", 30);
            //for every gaussian do...
            for (int g=0; g<gaussianCount; g++)
            {
                //for every data point do...
                for (unsigned int i=0; i < dataSize; i++)
                {
                    //calculate probability (non-normalized)
                    p(i,g) = gaussians[g]->calculateValue(data[i]);
                }
            }
            for (unsigned int i=0; i<dataSize; i++)
            {
                //normalize p.
                //resolve aliasing issues here with calling eval()
                p.row(i) = (p.row(i) / p.row(i).sum()).eval();
            }
            DEBUG_OUT("E-step END", 30);
            //E-step END
            
            //M-step BEGIN
            DEBUG_OUT("M-step BEGIN", 30);
            Eigen::VectorXd prob(gaussianCount);
            for (int g=0; g<gaussianCount; g++)
            {
                //calculate probabilities for a cluster
                prob(g) = p.col(g).mean();
                
                //calculate mu
                Eigen::VectorXd mu(dimension);
                mu.setZero();
                for (unsigned int i=0; i<dataSize; i++)
                {
                    //aliasing occurs, but does not harm.
                    mu = mu + (data[i] * p(i,g));
                }
                mu = mu / (dataSize * prob(g));
                
                //calculate sigma
                Eigen::MatrixXd sigma(dimension, dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    Eigen::VectorXd dist = data[i] - mu;
                    sigma = sigma + (p(i,g) * (dist * dist.transpose()));
                }
                sigma = sigma / (dataSize * prob(g));
                
                gaussians[g]->setMean(mu);
                gaussians[g]->setCovarianceMatrix(sigma);
            }
            DEBUG_OUT("M-step END", 30);
            //M-step END
            weights = prob;
            DEBUG_OUT("check for convergence... weights: " << weights << ", relative change of weights:" << (oldWeights - weights).norm() / oldWeights.norm(), 50);
            
            if ((oldWeights - weights).norm() / oldWeights.norm() < 10e-10)
                converged = true;
        }
        DEBUG_OUT("EM converged or terminated after " << iteration << " iterations...", 20);
        
        
        std::vector<std::pair<Gaussian*, double> > retVec;
        for (int g=0; g<gaussianCount; g++)
        {
            retVec.push_back(std::pair<Gaussian*, double>(gaussians[g], weights[g]));
        }
        return retVec;
        //TODO: return step
    }
    double GaussianMixtureModel::compareTo(const GaussianMixtureModel& other)
    {
        
    }
}

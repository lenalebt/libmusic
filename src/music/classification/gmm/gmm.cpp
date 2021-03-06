#include "gmm.hpp"

#include <Eigen/LU>
#include <assert.h>
#include "debug.hpp"
#include <set>
#include <limits>

namespace music
{
    
    /**
     * @todo precompute good starting vectors
     * @todo check result for being a local minimum
     */
    template <typename ScalarType>
    void GaussianMixtureModel<ScalarType>::trainGMM(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount, double initVariance, double minVariance)
    {
        //TODO: precompute good starting vectors.
        for (unsigned int i=0; i<gaussians.size(); i++)
            delete gaussians[i];
        gaussians.clear();
        gaussians = this->emAlg(std::vector<Gaussian<ScalarType>*>(), data, gaussianCount, 10, initVariance, minVariance);
        //TODO: check result for being a local minimum
    }
    
    /**
     * @bug This function does not work properly when you give it just a few data vectors.
     *      Seems to be a problem with linear dependent rows, as the covariance matricies are ill-conditioned
     *      (condition > 1e8).
     * @bug if the rank of a covariance matrix is zero, evil things happen.
     */
    template <typename ScalarType>
    std::vector<Gaussian<ScalarType>*> GaussianMixtureModelFullCov<ScalarType>::emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount, unsigned int maxIterations, double initVariance, double minVariance)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> > means;
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > fullCovs;
        
        unsigned int dimension = data[0].size();
        assert(dimension > 0);
        unsigned int dataSize = data.size();
        assert(dataSize > 0);
        assert(dataSize >= dimension);
        
        if (init.empty())
        {
            DEBUG_OUT("no init vectors given. using random values...", 20);
            //init with random data points and identity matricies as covariance matrix
            //first add points to a set, such that we have distinct init elements.
            std::set<unsigned int> initElements;
            UniformRNG<unsigned int> uniRNG(0, dataSize);
            while (initElements.size() < gaussianCount)
            {
                int newElement = uniRNG.rand();
                if (initElements.count(newElement) == 0)
                {
                    initElements.insert(newElement);
                }
            }
            //use the set to draw the elements
            for (std::set<unsigned int>::iterator it = initElements.begin(); it != initElements.end(); it++)
            {
                means.push_back(data[*it]);
                fullCovs.push_back(initVariance * Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>::Identity(dimension, dimension));
            }
        }
        else
        {
            DEBUG_OUT("init vectors given. using them...", 20);
            assert(init.size() == gaussianCount);
            //init with random data points and identity matricies as covariance matrix
            for (unsigned int g=0; g<gaussianCount; g++)
            {
                DEBUG_OUT("adding gaussian distribution " << g << "...", 25);
                
                means.push_back(init[g]->getMean());
                fullCovs.push_back(init[g]->getCovarianceMatrix());
            }
        }
        
        //set initial weights all equal to 1/gaussianCount
        Eigen::Matrix<double, Eigen::Dynamic, 1> weights = Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(gaussianCount, 1.0/double(gaussianCount));
        Eigen::Matrix<double, Eigen::Dynamic, 1> oldWeights = weights;
        Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic> p(dataSize, gaussianCount);
        float loglike=0.0, oldLoglike=0.0;
        
        unsigned int iteration = 0;
        bool converged = false;
        DEBUG_OUT("starting iteration loop...", 20);
        while ((iteration < maxIterations) && (!converged))
        {
            iteration++;
            oldWeights = weights;
            assert(weights.sum() > 0.95);
            assert(weights.sum() < 1.05);
            
            //E-step BEGIN
            DEBUG_OUT("E-step BEGIN", 30);
            //for every gaussian do...
            for (unsigned int g=0; g<gaussianCount; g++)
            {
                if ((fullCovs[g].diagonal().array() < minVariance).any())
                {
                    //if there is a coefficient that is smaller than minVariance in
                    // magnitude, set it to minVariance. This helps with bad results
                    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diag = fullCovs[g].diagonal();
                    for (int i=0; i<diag.size(); i++)
                    {
                        if (fabs(diag[i]) < minVariance)
                        {
                            fullCovs[g](i, i) = minVariance;
                        }
                    }
                }
                
                //DEBUG_VAR_OUT(fullCovs[g], 0);
                
                Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt(fullCovs[g]);
                //fullCovs[g].prod() is equal to its determinant for diagonal matricies.
                //double factor = 1.0/(pow(2*M_PI, dimension/2.0) * sqrt(fullCovs[g].template cast<double>().determinant()));
                double factor = -0.5*log(fullCovs[g].template cast<double>().determinant()) - (0.5*dimension) * log(2*M_PI);
                
                if (factor != factor)
                {
                    //the matrix is singular (determinant is zero, so factor is NaN)
                    //moore-penrose pseudo-inverse and pseudo-determinant will be used
                    //to calculate the value
                    //this is okay (degenerate case of multivariate gaussian)
                    
                    //calculate schur decomposition, which is an eigenvalue decomposition
                    //for the case of symmteric matricies.
                    //this is equivalent to a singular value decomposition since covariance
                    //matricies are positive semi-definite and thus is suited to calculate both
                    //moore-penrose pseudo-inverse and pseudo-determinant
                    Eigen::RealSchur<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> > rSchur(fullCovs[g].template cast<double>());
                    Eigen::Matrix<double, Eigen::Dynamic, 1> eigenvalues = rSchur.matrixT().diagonal();
                    Eigen::Matrix<double, Eigen::Dynamic, 1> invEigenvalues(eigenvalues.size());
                    
                    double logPseudoDet = 0.0;
                    int rank = 0;
                    for (int i=0; i<eigenvalues.size(); i++)
                    {
                        if (eigenvalues[i] > eigenvalues.size() * std::numeric_limits<kiss_fft_scalar>::epsilon())   //sum only nonzero eigenvalues. zero are values smaller than dimension*machineepsilon
                        {
                            logPseudoDet += log(eigenvalues[i]);
                            rank++;
                            invEigenvalues[i] = 1.0/eigenvalues[i];
                        }
                        else
                        {
                            invEigenvalues[i] = 0.0;
                        }
                    }
                    
                    //TODO: if rank==0, evil things happen, because one gaussian does not represent any samples.
                    
                    factor = -0.5*log(2.0*M_PI)*rank -0.5*logPseudoDet;
                    
                    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, Eigen::Dynamic> pseudoInverse =
                        (rSchur.matrixU() * invEigenvalues.asDiagonal() * rSchur.matrixU().transpose()).template cast<kiss_fft_scalar>()
                        ;
                    
                    //for every data point do...
                    for (unsigned int i=0; i < dataSize; i++)
                    {
                        //calculate probability (non-normalized)
                        //p(i,g) = factor * std::exp(-0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0));
                        p(i,g) = -0.5 * ((data[i] - means[g]).transpose() * pseudoInverse * (data[i] - means[g]))(0) + factor;
                    }
                }
                else
                {
                    //for every data point do...
                    for (unsigned int i=0; i < dataSize; i++)
                    {
                        //calculate probability (non-normalized)
                        //p(i,g) = factor * std::exp(-0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0));
                        p(i,g) = -0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0) + factor;
                    }
                }
            }
            
            //DEBUG_VAR_OUT(p, 0);
            
            double sum;
            double max;
            double logsum;
            oldLoglike = loglike;
            loglike=0.0;
            for (unsigned int i=0; i<dataSize; i++)
            {
                //normalize p.
                //use the log-exp-sum-trick from "Numerical Recipes", p.844-846
                max = p.row(i).maxCoeff();
                sum = (p.row(i) - max).exp().sum();
                logsum = log(sum);
                
                //resolve possible aliasing issues by calling eval().
                p.row(i) = (p.row(i) - (max + logsum)).exp().eval();
                
                loglike += max + logsum;
            }
            DEBUG_OUT("E-step END", 30);
            //E-step END
            
            //DEBUG_VAR_OUT(p, 0);
            
            //M-step BEGIN
            DEBUG_OUT("M-step BEGIN", 30);
            Eigen::Matrix<double, Eigen::Dynamic, 1> prob(gaussianCount);
            //calculate probabilities for all clusters
            prob = p.colwise().mean();
            DEBUG_VAR_OUT(prob.transpose(), 0);
            for (unsigned int g=0; g<gaussianCount; g++)
            {
                //calculate mu
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mu(dimension);
                mu.setZero();
                for (unsigned int i=0; i<dataSize; i++)
                {
                    //aliasing occurs, but does not harm.
                    mu.noalias() += (data[i] * p(i,g));
                }
                mu = mu / (dataSize * prob(g));
                
                //calculate sigma
                Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> sigma(dimension, dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    //sigma = sigma + (p(i,g) * (data[i] - mu).array().square()).matrix();
                    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = data[i] - mu;
                    sigma.noalias() += (p(i,g) * (dist * dist.transpose()));
                }
                sigma = sigma / (dataSize * prob(g));
                //DEBUG_VAR_OUT(sigma, 0);
                
                means[g] = mu;
                fullCovs[g] = sigma;
            }
            
            DEBUG_OUT("M-step END", 30);
            //M-step END
            weights = prob;
            DEBUG_OUT("check for convergence... weights: " << weights << ", relative change of weights:" << (oldWeights - weights).norm() / oldWeights.norm() << std::endl << "loglikechange: " << fabs(oldLoglike - loglike), 50);
            //if ((oldWeights - weights).norm() / oldWeights.norm() < 10e-6)
            //    converged = true;
            if (fabs(oldLoglike - loglike) < 1e-6)
                converged=true;
        }
        
        //get results with all-zero covariance matricies "right" (quick&dirty)
        for (unsigned int g=0; g<gaussianCount; g++)
        {
            if ((fullCovs[g].diagonal().array() < minVariance).any())
            {
                //if there is a coefficient that is smaller than minVariance in
                // magnitude, set it to minVariance. This helps with bad results
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diag = fullCovs[g].diagonal();
                for (int i=0; i<diag.size(); i++)
                {
                    if (fabs(diag[i]) < minVariance)
                    {
                        fullCovs[g](i, i) = minVariance;
                    }
                }
            }
        }
        
        if (converged)
        {
            DEBUG_OUT("EM converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("EM stopped after " << iteration << " iterations.", 20);
        }
        
        //calculate AIC, AICc, BIC
        this->calculateInformationCriteria(gaussianCount * (dimension + double(dimension*dimension+dimension)/(2.0)), dataSize, loglike);
        
        gaussians.clear();
        ScalarType sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (unsigned int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += weights[g];
            Gaussian<ScalarType>* gaussian = new GaussianFullCov<ScalarType>(dimension, normalRNG);
            gaussian->setMean(means[g]);
            gaussian->setCovarianceMatrix(fullCovs[g]);    //TODO: Improve this interface.
            gaussian->setWeight(weights[g]);
            normalizationFactor += gaussian->calculateValue(gaussian->getMean());
            
            gaussians.push_back(gaussian);
        }
        normalizationFactor /= gaussianCount;
        uniRNG = UniformRNG<ScalarType>(0.0, sumOfWeights);
        
        return gaussians;
    }
    
    template <typename ScalarType>
    std::vector<Gaussian<ScalarType>*> GaussianMixtureModelDiagCov<ScalarType>::emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount, unsigned int maxIterations, double initVariance, double minVariance)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> > means;
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> > diagCovs;
        std::vector<Gaussian<ScalarType>* > gaussians;
        
        unsigned int dimension = data[0].size();
        assert(dimension>0);
        unsigned int dataSize = data.size();
        assert(dataSize>0);
        assert(dataSize > gaussianCount);
        assert(dataSize >= dimension);
        
        if (init.empty())
        {
            DEBUG_OUT("no init vectors given. using random values...", 20);
            //init with random data points and identity matricies as covariance matrix
            //first add points to a set, such that we have distinct init elements.
            std::set<unsigned int> initElements;
            UniformRNG<unsigned int> uniRNG(0, dataSize);
            while (initElements.size() < gaussianCount)
            {
                int newElement = uniRNG.rand();
                if (initElements.count(newElement) == 0)
                {
                    initElements.insert(newElement);
                }
            }
            //use the set to draw the elements
            for (std::set<unsigned int>::iterator it = initElements.begin(); it != initElements.end(); it++)
            {
                means.push_back(data[*it]);
                diagCovs.push_back(Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Constant(dimension, 1, initVariance));
            }
        }
        else
        {
            DEBUG_OUT("init vectors given. using them...", 20);
            assert(init.size() == gaussianCount);
            //init with random data points and identity matricies as covariance matrix
            for (unsigned int g=0; g<gaussianCount; g++)
            {
                DEBUG_OUT("adding gaussian distribution " << g << "...", 25);
                
                means.push_back(init[g]->getMean());
                diagCovs.push_back(init[g]->getCovarianceMatrix());
            }
        }
        
        //set initial weights all equal to 1/gaussianCount
        Eigen::Matrix<double, Eigen::Dynamic, 1> weights = Eigen::Matrix<double, Eigen::Dynamic, 1>::Constant(gaussianCount, 1.0/double(gaussianCount));
        Eigen::Matrix<double, Eigen::Dynamic, 1> oldWeights = weights;
        Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic> p(dataSize, gaussianCount);
        double loglike=0.0, oldLoglike=0.0;
        
        unsigned int iteration = 0;
        bool converged = false;
        DEBUG_OUT("starting iteration loop...", 20);
        while ((iteration < maxIterations) && (!converged))
        {
            iteration++;
            oldWeights = weights;
            assert(weights.sum() > 0.95);
            assert(weights.sum() < 1.05);
            
            //E-step BEGIN
            DEBUG_OUT("E-step BEGIN", 30);
            //for every gaussian do...
            for (unsigned int g=0; g<gaussianCount; g++)
            {
                //if there is a coefficient that is smaller than minVariance in
                // magnitude, set it to minVariance. This helps with bad results
                for (int i=0; i<diagCovs[g].size(); i++)
                {
                    if (fabs(diagCovs[g][i]) < minVariance)
                    {
                        diagCovs[g][i] = minVariance;
                    }
                }
                //Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt(diagCovs[g].asDiagonal());
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diagCovInv = diagCovs[g].array().inverse();
                //diagCovs[g].prod() is equal to its determinant for diagonal matricies.
                //taking the sqrt first helps with some accuracy issues
                //double factor = 1.0/( pow(2*M_PI, dimension/2.0) * diagCovs[g].template cast<double>().cwiseSqrt().prod() );
                
                //alternative version with logarithms. log of determinant is sum of logs, since it is a log of a product.
                double factor = -0.5 * diagCovs[g].array().log().sum() - (0.5*dimension) * log(2*M_PI);
                
                //for every data point do...
                for (unsigned int i=0; i < dataSize; i++)
                {
                    //calculate probability (non-normalized)
                    //first version: with ldlt-solver. second version: optimized for diagonal matricies. third version: diagonal matricies with logarithms
                    //p(i,g) = factor * std::exp(-0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0));
                    //p(i,g) = factor * std::exp(-0.5 * ((data[i] - means[g]).transpose() * (diagCovInv.array() * (data[i] - means[g]).array() ).matrix() )(0));
                    p(i,g) = -0.5 * ((data[i] - means[g]).transpose() * (diagCovInv.array() * (data[i] - means[g]).array() ).matrix() )(0) + factor;
                }
            }
            
            //DEBUG_VAR_OUT(p, 0);
            
            double sum;
            double max;
            double logsum;
            oldLoglike = loglike;
            loglike=0.0;
            for (unsigned int i=0; i<dataSize; i++)
            {
                //normalize p.
                //use the log-exp-sum-trick from "Numerical Recipes", p.844-846
                max = p.row(i).maxCoeff();
                sum = (p.row(i) - max).exp().sum();
                logsum = log(sum);
                
                //resolve possible aliasing issues by calling eval().
                p.row(i) = (p.row(i) - (max + logsum)).exp().eval();
                
                loglike += max + logsum;
            }
            DEBUG_OUT("E-step END", 30);
            //E-step END
            
            //DEBUG_VAR_OUT(p, 0);
            
            //M-step BEGIN
            DEBUG_OUT("M-step BEGIN", 30);
            Eigen::Matrix<double, Eigen::Dynamic, 1> prob(gaussianCount);
            //calculate probabilities for all clusters
            prob = p.colwise().mean();
            for (unsigned int g=0; g<gaussianCount; g++)
            {
                //calculate mu
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mu(dimension);
                mu.setZero();
                for (unsigned int i=0; i<dataSize; i++)
                {
                    //aliasing occurs, but does not harm.
                    mu.noalias() += (data[i] * p(i,g));
                }
                mu = mu / (dataSize * prob(g));
                
                //calculate sigma
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> sigma(dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    sigma.noalias() += (p(i,g) * (data[i] - mu).array().square()).matrix();
                }
                sigma = sigma / (dataSize * prob(g));
                
                means[g] = mu;
                diagCovs[g] = sigma;
            }
            
            DEBUG_OUT("M-step END", 30);
            //M-step END
            weights = prob;
            DEBUG_OUT("check for convergence... weights: " << weights << ", relative change of weights:" << (oldWeights - weights).norm() / oldWeights.norm() << std::endl << "loglikechange: " << fabs(oldLoglike - loglike), 50);
            //if ((oldWeights - weights).norm() / oldWeights.norm() < 10e-6)
            //    converged = true;
            if (fabs(oldLoglike - loglike) < 1e-6)
                converged=true;
        }
        
        //get results with all-zero covariance matricies "right" (quick&dirty)
        for (unsigned int g=0; g<gaussianCount; g++)
        {
            //if there is a coefficient that is smaller than minVariance in
            // magnitude, set it to minVariance. This helps with bad results
            for (int i=0; i<diagCovs[g].size(); i++)
            {
                if (fabs(diagCovs[g][i]) < minVariance)
                {
                    diagCovs[g][i] = minVariance;
                }
            }
        }
        
        if (converged)
        {
            DEBUG_OUT("EM converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("EM stopped after " << iteration << " iterations.", 20);
        }
        
        //calculate AIC, AICc, BIC
        this->calculateInformationCriteria(gaussianCount * 2*dimension, dataSize, loglike);
        
        gaussians.clear();
        ScalarType sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (unsigned int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += weights[g];
            Gaussian<ScalarType>* gaussian = new GaussianDiagCov<ScalarType>(dimension, normalRNG);
            gaussian->setMean(means[g]);
            gaussian->setCovarianceMatrix(diagCovs[g].asDiagonal());    //TODO: Improve this interface.
            gaussian->setWeight(weights[g]);
            normalizationFactor += gaussian->calculateValue(gaussian->getMean());
            
            gaussians.push_back(gaussian);
        }
        normalizationFactor /= gaussianCount;
        uniRNG = UniformRNG<ScalarType>(0.0, sumOfWeights);
        
        return gaussians;
    }
    
    template <typename ScalarType>
    void GaussianMixtureModel<ScalarType>::calculateInformationCriteria(int k, int n, double loglike)
    {
        double N=n;
        double K=k;
        this->loglike = loglike;
        this->aic = 2*K - 2*loglike;
        this->aicc = aic + (2*K*(K+1))/(N-K-1);
        this->bic = K*log(n) - 2*loglike;
        
        DEBUG_OUT("model information: information criteria", 50);
        DEBUG_VAR_OUT(k,        5);
        DEBUG_VAR_OUT(n,        5);
        DEBUG_VAR_OUT(loglike,  5);
        DEBUG_VAR_OUT(aic,      5);
        DEBUG_VAR_OUT(aicc,     5);
        DEBUG_VAR_OUT(bic,      5);
    }
    
    template <typename ScalarType>
    ScalarType GaussianMixtureModel<ScalarType>::calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& pos) const
    {
        assert (gaussians.size() > 0);
        ScalarType retVal=0.0;
        for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            retVal += (**it).calculateValue(pos);
        }
        return retVal;
    }
    /**
     * @todo entfernen, falls nicht mehr gebraucht, oder dokumentieren, falls doch
     */
    template <typename ScalarType>
    ScalarType GaussianMixtureModel<ScalarType>::calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& pos) const
    {
        assert (gaussians.size() > 0);
        ScalarType retVal=0.0;
        for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            retVal += (**it).calculateValueWithoutWeights(pos);
        }
        return retVal/gaussians.size();
    }
    
    template <typename ScalarType>
    ScalarType GaussianMixtureModel<ScalarType>::compareTo(const GaussianMixtureModel<ScalarType>& other, int sampleCount)
    {
        //draw some samples from the one model and take a look at the pdf values of the other.
        double value = 0.0;
        
        UniformRNG<float> rng(0,1);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> tmp;
        double tmpVal;
        
        for (int i=0; i<sampleCount; i++)
        {
            
            /*
            //L2-Distance of functions
            if (rng.rand() < 0.5)
            {
                tmp = this->rand();
                tmpVal = this->calculateValue(tmp);
            }
            else
            {
                tmp = other.rand();
                tmpVal = other.calculateValue(tmp);
            }
            value += fabs(this->calculateValue(tmp) - other.calculateValue(tmp)) / tmpVal;
            */
            
            //Kullback-Leibler-Divergence
            tmp = this->rand();
            tmpVal = this->calculateValue(tmp);
            assert(tmpVal > 0);
            assert(tmpVal == tmpVal);
            //DEBUG_VAR_OUT(tmpVal, 0);
            double logTmpVal = log(tmpVal);
            //do not allow +-inf values...
            if (logTmpVal < -100)
                logTmpVal = -100;
            //if (logTmpVal > 100)
            //    logTmpVal = 100;
            double logOtherVal = log(other.calculateValue(tmp));
            assert(logOtherVal == logOtherVal);
            if (logOtherVal < -100)
                logOtherVal = -100;
            //if (logOtherVal > 100)
            //    logOtherVal = 100;
            value += 1.0 * (logTmpVal - logOtherVal);       //1.0 will be optimized out by the compiler
            
            assert(value == value);
        }
        assert(value/sampleCount == value/sampleCount);
        return value/sampleCount;
    }
    
    template <typename ScalarType>
    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> GaussianMixtureModel<ScalarType>::rand() const
    {
        assert(gaussians.size() > 0);
        
        ScalarType sumOfWeights=0.0;
        ScalarType randomNumber = uniRNG.rand();
        for (unsigned int i=0; i<gaussians.size(); i++)
        {
            sumOfWeights += gaussians[i]->getWeight();
            if (sumOfWeights >= randomNumber)
                return gaussians[i]->rand();
        }
        
        //this should never happen...
        ERROR_OUT("You should never see this, as you will not get random numbers outside of [0,1[ from a random number generator generating numbers in [0,1[. If you see this, something veeery weird happened.", 0);
        return gaussians[0]->rand();
    }
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>::GaussianMixtureModel() :
        gaussians(), uniRNG(0.0, 1.0), normalizationFactor(1.0),
        aic(0.0), aicc(0.0), bic(0.0), loglike(0.0)
    {
        
    }
    
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>::GaussianMixtureModel(const GaussianMixtureModel<ScalarType>& other) :
        gaussians(), uniRNG(0.0, 1.0), normalizationFactor(other.normalizationFactor),
        aic(other.aic), aicc(other.aicc), bic(other.bic), loglike(other.loglike)
    {
        for (unsigned int i=0; i<other.gaussians.size(); i++)
        {
            gaussians.push_back(other.gaussians[i]->clone());
        }
        assert(gaussians.size() == other.gaussians.size());
    }
    
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>::~GaussianMixtureModel()
    {
        for (unsigned int i=0; i<gaussians.size(); i++)
        {
            delete gaussians[i];
        }
        gaussians.clear();
    }
    
    template <typename ScalarType>
    GaussianMixtureModelFullCov<ScalarType>::GaussianMixtureModelFullCov(const GaussianMixtureModelFullCov<ScalarType>& other) :
        GaussianMixtureModel<ScalarType>(other)
    {
        assert(gaussians.size() == other.gaussians.size());
    }
    template <typename ScalarType>
    GaussianMixtureModelDiagCov<ScalarType>::GaussianMixtureModelDiagCov(const GaussianMixtureModelDiagCov<ScalarType>& other) :
        GaussianMixtureModel<ScalarType>(other)
    {
        assert(gaussians.size() == other.gaussians.size());
    }
    
    template <typename ScalarType>
    GaussianMixtureModelFullCov<ScalarType>::GaussianMixtureModelFullCov() :
        GaussianMixtureModel<ScalarType>()
    {
        
    }
    template <typename ScalarType>
    GaussianMixtureModelDiagCov<ScalarType>::GaussianMixtureModelDiagCov() :
        GaussianMixtureModel<ScalarType>()
    {
        
    }
    
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>* GaussianMixtureModelDiagCov<ScalarType>::clone()
    {
        return new GaussianMixtureModelDiagCov<ScalarType>(*this);
    }
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>* GaussianMixtureModelFullCov<ScalarType>::clone()
    {
        return new GaussianMixtureModelFullCov<ScalarType>(*this);
    }
    
    template <typename ScalarType>
    std::string GaussianMixtureModel<ScalarType>::toJSONString(bool styledWriter) const
    {
        DEBUG_OUT("saving model as JSON", 20);
        //uses Jsoncpp as library. Jsoncpp is licensed as MIT, so we may use it without restriction.
        Json::Value root(Json::arrayValue);
        Json::Writer* writer=NULL;
        
        if (styledWriter)
            writer = new Json::StyledWriter();
        else
            writer = new Json::FastWriter();
        
        for (unsigned int g=0; g<gaussians.size(); g++)
        {
            Json::Value arrayElement;
            
            //output the weight
            arrayElement["weight"]      = gaussians[g]->getWeight();
            
            //output the mean as array of doubles
            Json::Value mean(Json::arrayValue);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> gMean = gaussians[g]->getMean();
            for (int i=0; i<gMean.size(); i++)
                mean.append(Json::Value(gMean[i]));
            arrayElement["mean"]        = mean;
            
            //output the variance as array of double. only save the lower
            //triangular, as the other values are mirrored.
            Json::Value variance(Json::arrayValue);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> gVariance = gaussians[g]->getCovarianceMatrix();
            if (isDiagonal(gVariance))
            {
                DEBUG_OUT("save as diagonal covariance matrix", 30);
                for (int i=0; i<gVariance.rows(); i++)
                {
                    variance.append(Json::Value(gVariance(i, i)));
                }
            }
            else
            {
                DEBUG_OUT("save as full covariance matrix", 30);
                for (int i=0; i<gVariance.rows(); i++)
                {
                    for (int j=i; j<gVariance.cols(); j++)
                    {
                        variance.append(Json::Value(gVariance(i, j)));
                    }
                }
            }
            arrayElement["covariance"]    = variance;
            
            root.append(arrayElement);
        }
        
        std::string str = writer->write(root);
        delete writer;
        return str;
    }
    
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>* GaussianMixtureModel<ScalarType>::loadFromJSONString(const std::string& jsonString)
    {
        if (jsonString != "")
        {
            Json::Value root;
            Json::Reader reader;
            reader.parse(jsonString, root, false);
            return loadFromJsonValue(root);
        }
        else
            return NULL;
    }
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>* GaussianMixtureModel<ScalarType>::loadFromJsonValue(Json::Value& jsonValue)
    {
        DEBUG_OUT("load model from JSON...", 20);
        GaussianMixtureModel<ScalarType>* gmm = NULL;
        
        //read gaussians from json input.
        for (unsigned int g=0; g<jsonValue.size(); g++)
        {
            //first init variables and read dimensionality
            Gaussian<ScalarType>* gauss=Gaussian<ScalarType>::loadFromJsonValue(jsonValue[g]);
            
            if (gmm == NULL)
            {
                if (gauss->isFullCov())
                {
                    gmm = new GaussianMixtureModelFullCov<ScalarType>();
                }
                else
                {
                    gmm = new GaussianMixtureModelDiagCov<ScalarType>();
                }
            }
            
            gmm->gaussians.push_back(gauss);
        }
        
        //set normalization factor
        double sumOfWeights = 0.0;
        double normalizationFactor = 0.0;
        for (unsigned int g=0; g<gmm->gaussians.size(); g++)
        {
            sumOfWeights += gmm->gaussians[g]->getWeight();
            normalizationFactor += gmm->gaussians[g]->calculateValue(gmm->gaussians[g]->getMean());
        }
        gmm->normalizationFactor /= gmm->gaussians.size();
        
        return gmm;
    }
    
    template <typename ScalarType>
    std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<ScalarType>& model)
    {
        //use a fast writer instead of a styled writer, produces smaller JSON
        return (os << model.toJSONString(false));
    }
    
    template <typename ScalarType>
    std::istream& operator>>(std::istream& is, GaussianMixtureModel<ScalarType>& model)
    {
        Json::Value root;
        Json::Reader reader;
        reader.parse(is, root, false);
        model.loadFromJsonValue(root);
        return is;
    }
    
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>* GaussianMixtureModel<ScalarType>::operator+(const GaussianMixtureModel<ScalarType>& other)
    {
        //will take the gaussians of the two gmms and mix them.
        GaussianMixtureModel<ScalarType>* newgmm = NULL;
        std::vector<Gaussian<ScalarType>*> gaussians1 = this->getGaussians();
        std::vector<Gaussian<ScalarType>*> gaussians2 = other.getGaussians();
        
        //determine wether or not the matricies are all diagonal.
        bool fullCov = false;
        for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians1.begin(); it != gaussians1.end(); it++)
        {
            fullCov |= !isDiagonal((*it)->getCovarianceMatrix());
        }
        for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians2.begin(); it != gaussians2.end(); it++)
        {
            fullCov |= !isDiagonal((*it)->getCovarianceMatrix());
        }
        
        //okay, two processes for different matricies...
        if (fullCov)
        {
            newgmm = new GaussianMixtureModelFullCov<ScalarType>();
            GaussianFullCov<ScalarType>* full = NULL;
            for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians1.begin(); it != gaussians1.end(); it++)
            {
                full = new GaussianFullCov<ScalarType>((*it)->getWeight(), (*it)->getMean());
                full->setCovarianceMatrix((*it)->getCovarianceMatrix());
                newgmm->gaussians.push_back(full);
            }
            for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians2.begin(); it != gaussians2.end(); it++)
            {
                full = new GaussianFullCov<ScalarType>((*it)->getWeight(), (*it)->getMean());
                full->setCovarianceMatrix((*it)->getCovarianceMatrix());
                newgmm->gaussians.push_back(full);
            }
        }
        else
        {
            newgmm = new GaussianMixtureModelDiagCov<ScalarType>();
            GaussianDiagCov<ScalarType>* diag = NULL;
            for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians1.begin(); it != gaussians1.end(); it++)
            {
                diag = new GaussianDiagCov<ScalarType>((*it)->getWeight(), (*it)->getMean());
                diag->setCovarianceMatrix((*it)->getCovarianceMatrix());
                newgmm->gaussians.push_back(diag);
            }
            for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians2.begin(); it != gaussians2.end(); it++)
            {
                diag = new GaussianDiagCov<ScalarType>((*it)->getWeight(), (*it)->getMean());
                diag->setCovarianceMatrix((*it)->getCovarianceMatrix());
                newgmm->gaussians.push_back(diag);
            }
        }
        
        //set normalization factor
        double sumOfWeights = 0.0;
        newgmm->normalizationFactor = 0.0;
        int gaussianCount = gaussians1.size()+gaussians2.size();
        for (int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += newgmm->gaussians[g]->getWeight();
            newgmm->normalizationFactor += newgmm->gaussians[g]->calculateValue(newgmm->gaussians[g]->getMean());
        }
        newgmm->normalizationFactor /= gaussianCount;
        
        return newgmm;
    }
    
    
    
    
    template class GaussianMixtureModel<kiss_fft_scalar>;
    template class GaussianMixtureModelFullCov<kiss_fft_scalar>;
    template class GaussianMixtureModelDiagCov<kiss_fft_scalar>;
    
    template <> NormalRNG<kiss_fft_scalar>* GaussianMixtureModel<kiss_fft_scalar>::normalRNG = new NormalRNG<kiss_fft_scalar>();
    
    template std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<kiss_fft_scalar>& model);
    template std::istream& operator>>(std::istream& is, GaussianMixtureModel<kiss_fft_scalar>& model);
    
}

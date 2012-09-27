#include "gmm.hpp"

#include <Eigen/LU>
#include <assert.h>
#include "debug.hpp"
#include <set>

namespace music
{
    #define SMALLEST_VARIANCE_VALUE 0.1
    
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(unsigned int dimension, NormalRNG<ScalarType>* rng) :
        weight(1.0), mean(dimension), preFactor(), rng(rng), externalRNG(rng!=NULL)
    {
        if (!this->rng)
            this->rng = new NormalRNG<ScalarType>();
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng) :
        weight(weight), mean(mean), preFactor(), rng(rng), externalRNG(rng!=NULL)
    {
        assert(weight >= 0.0);
        assert(weight <= 1.0);
        
        if (!this->rng)
            this->rng = new NormalRNG<ScalarType>();
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(const Gaussian<ScalarType>& other) :
        weight(other.weight), mean(other.mean), preFactor(), rng(NULL), externalRNG(other.externalRNG)
    {
        if (externalRNG)
            this->rng = other.rng;
        if (!this->rng)
            this->rng = new NormalRNG<ScalarType>();
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>::~Gaussian()
    {
        if (!externalRNG)
            delete rng;
    }
    
    template <typename ScalarType>
    double GaussianDiagCov<ScalarType>::calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        //Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist2 = dist;
        return preFactor * std::exp(-0.5 * (dist.transpose() * ldlt.solve(dist))(0));
    }
    template <typename ScalarType>
    double GaussianDiagCov<ScalarType>::calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        //Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist2 = dist;
        return preFactorWithoutWeights * std::exp(-0.5 * (dist.transpose() * ldlt.solve(dist))(0));
    }
    template <typename ScalarType>
    double GaussianDiagCov<ScalarType>::calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        return preFactor * std::exp(-0.5 * (dataVector.transpose() * ldlt.solve(dataVector))(0));
    }
    template <typename ScalarType>
    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> GaussianDiagCov<ScalarType>::rand() const
    {
        assert(mean.size() > 0);
        Eigen::LLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > llt(diagCov.asDiagonal());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> y(mean.size());
        for (int i=0; i<mean.size(); i++)
            y[i] = rng->rand();
        return llt.matrixL() * y + mean;
    }
    template <typename ScalarType>
    void GaussianDiagCov<ScalarType>::calculatePrefactor()
    {
        assert(diagCov.cwiseSqrt().prod() != 0.0);
        preFactor = weight * 1.0/(pow(2*M_PI, diagCov.size()/2.0) * diagCov.cwiseSqrt().prod());
        assert(preFactor != 0.0);
        preFactorWithoutWeights = 1.0/(pow(2*M_PI, diagCov.size()/2.0) * diagCov.cwiseSqrt().prod());
        assert(preFactorWithoutWeights != 0.0);
    }
    template <typename ScalarType>
    void GaussianDiagCov<ScalarType>::setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        this->diagCov = fullCov.diagonal();
        ldlt.compute(diagCov.asDiagonal());
        
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(unsigned int dimension, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(dimension, rng)
    {
        assert(this->rng != NULL);
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(weight, mean, rng), diagCov(mean.size())
    {
        assert(this->rng != NULL);
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(const GaussianDiagCov<ScalarType>& other) :
        Gaussian<ScalarType>(other), diagCov(other.diagCov)
    {
        assert(this->rng != NULL);
        calculatePrefactor();
        ldlt.compute(diagCov.asDiagonal());
    }
    
    template <typename ScalarType>
    void GaussianFullCov<ScalarType>::calculatePrefactor()
    {
        assert(fullCov.determinant() != 0.0);
        preFactor = weight * 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
        assert(preFactor != 0.0);
        preFactorWithoutWeights = 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
        assert(preFactorWithoutWeights != 0.0);
    }
    template <typename ScalarType>
    void GaussianFullCov<ScalarType>::setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        this->fullCov = fullCov;
        ldlt.compute(fullCov);
        
        calculatePrefactor();
    }
    template <typename ScalarType>
    double GaussianFullCov<ScalarType>::calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        return preFactor * std::exp(-0.5 * (dist.transpose() * ldlt.solve(dist))(0));
    }
    template <typename ScalarType>
    double GaussianFullCov<ScalarType>::calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        return preFactorWithoutWeights * std::exp(-0.5 * (dist.transpose() * ldlt.solve(dist))(0));
    }
    template <typename ScalarType>
    double GaussianFullCov<ScalarType>::calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        return preFactor * std::exp(-0.5 * (dataVector.transpose() * ldlt.solve(dataVector))(0));
    }
    template <typename ScalarType>
    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> GaussianFullCov<ScalarType>::rand() const
    {
        assert(mean.size() > 0);
        Eigen::LLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > llt(fullCov);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> y(mean.size());
        for (int i=0; i<mean.size(); i++)
            y[i] = rng->rand();
        return llt.matrixL() * y + mean;
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(unsigned int dimension, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(dimension, rng), fullCov(dimension, dimension), ldlt()
    {
        assert(this->rng != NULL);
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(weight, mean, rng), fullCov(mean.size(), mean.size())
    {
        assert(this->rng != NULL);
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(const GaussianFullCov<ScalarType>& other) :
        Gaussian<ScalarType>(other), fullCov(other.fullCov)
    {
        assert(this->rng != NULL);
        calculatePrefactor();
        ldlt.compute(fullCov);
    }
    
    /**
     * @todo precompute good starting vectors
     * @todo check result for being a local minimum
     */
    template <typename ScalarType>
    void GaussianMixtureModel<ScalarType>::trainGMM(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount)
    {
        //TODO: precompute good starting vectors.
        gaussians = this->emAlg(std::vector<Gaussian<ScalarType>*>(), data, gaussianCount, 10);
        //TODO: check result for being a local minimum
    }
    
    /**
     * @bug This function does not work properly when you give it just a few data vectors.
     *      Seems to be a problem with linear dependent rows, as the covariance matricies are ill-conditioned
     *      (condition > 1e8).
     */
    template <typename ScalarType>
    std::vector<Gaussian<ScalarType>*> GaussianMixtureModelFullCov<ScalarType>::emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount, unsigned int maxIterations)
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
                fullCovs.push_back(100.0 * Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>::Identity(dimension, dimension));
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
                if ((fullCovs[g].diagonal().array() < SMALLEST_VARIANCE_VALUE).any())
                {
                    //if there is a coefficient that is smaller than SMALLEST_VARIANCE_VALUE in
                    // magnitude, set it to SMALLEST_VARIANCE_VALUE. This helps with bad results
                    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diag = fullCovs[g].diagonal();
                    for (int i=0; i<diag.size(); i++)
                    {
                        if (fabs(diag[i]) < SMALLEST_VARIANCE_VALUE)
                        {
                            fullCovs[g](i, i) = SMALLEST_VARIANCE_VALUE;
                        }
                    }
                }
                
                Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt(fullCovs[g]);
                //fullCovs[g].prod() is equal to its determinant for diagonal matricies.
                //double factor = 1.0/(pow(2*M_PI, dimension/2.0) * sqrt(fullCovs[g].template cast<double>().determinant()));
                double factor = -0.5*log(fullCovs[g].template cast<double>().determinant()) - (0.5*dimension) * log(2*M_PI);
                
                /*if (factor != factor)
                {
                    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diag = fullCovs[g].diagonal();
                    fullCovs[g].setZero();
                    fullCovs[g].diagonal() = diag;
                    factor = 1.0/(pow(2*M_PI, dimension/2.0) * sqrt(fullCovs[g].template cast<double>().determinant()));
                    DEBUG_OUT("lalala, sachen ersetzt!", 10);
                }*/
                
                //for every data point do...
                for (unsigned int i=0; i < dataSize; i++)
                {
                    //calculate probability (non-normalized)
                    //p(i,g) = factor * std::exp(-0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0));
                    p(i,g) = -0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0) + factor;
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
                Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> sigma(dimension, dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    //sigma = sigma + (p(i,g) * (data[i] - mu).array().square()).matrix();
                    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = data[i] - mu;
                    sigma.noalias() += (p(i,g) * (dist * dist.transpose()));
                }
                sigma = sigma / (dataSize * prob(g));
                
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
        
        //get results with all-zero covariance matricies right (quick&dirty)
        for (unsigned int g=0; g<gaussianCount; g++)
        {
            if ((fullCovs[g].diagonal().array() < SMALLEST_VARIANCE_VALUE).any())
            {
                //if there is a coefficient that is smaller than SMALLEST_VARIANCE_VALUE in
                // magnitude, set it to SMALLEST_VARIANCE_VALUE. This helps with bad results
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> diag = fullCovs[g].diagonal();
                for (int i=0; i<diag.size(); i++)
                {
                    if (fabs(diag[i]) < SMALLEST_VARIANCE_VALUE)
                    {
                        fullCovs[g](i, i) = SMALLEST_VARIANCE_VALUE;
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
    std::vector<Gaussian<ScalarType>*> GaussianMixtureModelDiagCov<ScalarType>::emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int gaussianCount, unsigned int maxIterations)
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
                diagCovs.push_back(Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Constant(dimension, 1, 100.0));
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
                //if there is a coefficient that is smaller than SMALLEST_VARIANCE_VALUE in
                // magnitude, set it to SMALLEST_VARIANCE_VALUE. This helps with bad results
                for (int i=0; i<diagCovs[g].size(); i++)
                {
                    if (fabs(diagCovs[g][i]) < SMALLEST_VARIANCE_VALUE)
                    {
                        diagCovs[g][i] = SMALLEST_VARIANCE_VALUE;
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
        
        //get results with all-zero covariance matricies right (quick&dirty)
        for (unsigned int g=0; g<gaussianCount; g++)
        {
            //if there is a coefficient that is smaller than SMALLEST_VARIANCE_VALUE in
            // magnitude, set it to SMALLEST_VARIANCE_VALUE. This helps with bad results
            for (int i=0; i<diagCovs[g].size(); i++)
            {
                if (fabs(diagCovs[g][i]) < SMALLEST_VARIANCE_VALUE)
                {
                    diagCovs[g][i] = SMALLEST_VARIANCE_VALUE;
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
    ScalarType GaussianMixtureModel<ScalarType>::compareTo(const GaussianMixtureModel<ScalarType>& other)
    {
        //draw some samples from the one model and take a look at the pdf values of the other.
        double value = 0.0;
        double value2=0.0;
        const int numValues = 50;
        
        UniformRNG<float> rng(0,1);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> tmp;
        double tmpVal, tmp2Val;
        
        for (int i=0; i<numValues; i++)
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
            //DEBUG_VAR_OUT(logTmpVal, 0);
            //DEBUG_VAR_OUT(logOtherVal, 0);
            //DEBUG_VAR_OUT(logTmpVal - logOtherVal, 0);
            value += tmpVal * (logTmpVal - logOtherVal);
            value2 += 1 * (logTmpVal - logOtherVal);
            //std::cerr << "tmp: " << tmpVal << std::endl;
            //std::cerr << "otp: " << other.calculateValue(tmp) << std::endl;
            //std::cerr << "dif: " << logTmpVal - logOtherVal << std::endl;
            //std::cerr << "t  : " << tmpVal * (logTmpVal - logOtherVal) << std::endl;;
            
            //DEBUG_VAR_OUT(value, 0);
            assert(value == value);
            assert(value2 == value2);
        }
        assert(value2/numValues == value2/numValues);
        //std::cerr << value2/numValues << std::endl;
        return value2/numValues;
        
        //get the mean of the pdf values. large values should mean "pdfs are similar",
        //and small values indicate that they are not equal.
        for (int i=0; i<numValues; i++)
            value += other.calculateValue(this->rand());
        value /= numValues * normalizationFactor;
        return value;
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
        gaussians(), uniRNG(0.0, 1.0), normalizationFactor(1.0)
    {
        
    }
    
    template <typename ScalarType>
    GaussianMixtureModel<ScalarType>::GaussianMixtureModel(const GaussianMixtureModel<ScalarType>& other) :
        gaussians(), uniRNG(0.0, 1.0), normalizationFactor(other.normalizationFactor)
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
    Gaussian<ScalarType>* GaussianDiagCov<ScalarType>::clone()
    {
        return new GaussianDiagCov<ScalarType>(*this);
    }
    template <typename ScalarType>
    Gaussian<ScalarType>* GaussianFullCov<ScalarType>::clone()
    {
        return new GaussianFullCov<ScalarType>(*this);
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
                DEBUG_OUT("save as diagonal covariance matrix", 10);
                for (int i=0; i<gVariance.rows(); i++)
                {
                    variance.append(Json::Value(gVariance(i, i)));
                }
            }
            else
            {
                DEBUG_OUT("save as full covariance matrix", 10);
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
    void GaussianMixtureModel<ScalarType>::loadFromJSONString(const std::string& jsonString)
    {
        Json::Value root;
        Json::Reader reader;
        reader.parse(jsonString, root, false);
        loadFromJsonValue(root);
    }
    template <typename ScalarType>
    void GaussianMixtureModel<ScalarType>::loadFromJsonValue(Json::Value& jsonValue)
    {
        DEBUG_OUT("load model from JSON...", 20);
        //first: empty list of old gaussians.
        for (unsigned int g=0; g<gaussians.size(); g++)
            delete gaussians[g];
        gaussians.clear();
        
        //read gaussians from json input.
        for (unsigned int g=0; g<jsonValue.size(); g++)
        {
            //first init variables and read dimensionality
            Gaussian<ScalarType>* gauss=NULL;
            int dimension=jsonValue[g]["mean"].size();
            bool varianceIsFull;
            if (jsonValue[g]["mean"].size() == jsonValue[g]["covariance"].size())
            {
                DEBUG_OUT("loading as diagonal covariance matrix", 30);
                gauss = new GaussianDiagCov<ScalarType>(dimension);
                varianceIsFull = false;
            }
            else
            {
                DEBUG_OUT("loading as full covariance matrix", 30);
                gauss = new GaussianFullCov<ScalarType>(dimension);
                varianceIsFull = true;
            }
            
            //read weight
            gauss->setWeight(jsonValue[g]["weight"].asDouble());
            
            //read mean
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean(dimension);
            Json::Value jMean = jsonValue[g]["mean"];
            for (int i=0; i<dimension; i++)
                mean[i] = jMean[i].asDouble();
            gauss->setMean(mean);
            
            //read covariance
            Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> variance(dimension, dimension);
            Json::Value jVariance = jsonValue[g]["covariance"];
            if (varianceIsFull)
            {
                //read values in lower triangular order.
                //i and j are the coordinates of the matrix (i=row, j=col)
                //k is the index of the input vector.
                int i=0;
                int j=0;
                for (unsigned int k=0; k<jVariance.size(); k++)
                {
                    variance(i,j) = jVariance[k].asDouble();
                    if (i != j)
                        variance(j,i) = variance(i,j);
                    
                    //go one step to the right when we exceed the matrix dimensions
                    //begin on the diagonal.
                    i++;
                    if (i>=dimension)
                    {
                        j++;
                        i=j;
                    }
                }
            }
            else
            {
                //set all Values to zero, as we will not write to every cell.
                variance.setZero();
                //read diagonal elements. we don't have other values.
                for (int i=0; i<dimension; i++)
                    variance(i,i) = jVariance[i].asDouble();
            }
            gauss->setCovarianceMatrix(variance);
            
            gaussians.push_back(gauss);
        }
        
        //set normalization factor
        double sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (unsigned int g=0; g<gaussians.size(); g++)
        {
            sumOfWeights += gaussians[g]->getWeight();
            normalizationFactor += gaussians[g]->calculateValue(gaussians[g]->getMean());
        }
        normalizationFactor /= gaussians.size();
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
    
    
    
    
    template class GaussianFullCov<double>;
    template class GaussianDiagCov<double>;
    template class GaussianFullCov<float>;
    template class GaussianDiagCov<float>;
    template class GaussianMixtureModel<kiss_fft_scalar>;
    template class GaussianMixtureModelFullCov<kiss_fft_scalar>;
    template class GaussianMixtureModelDiagCov<kiss_fft_scalar>;
    
    template <> NormalRNG<kiss_fft_scalar>* GaussianMixtureModel<kiss_fft_scalar>::normalRNG = new NormalRNG<kiss_fft_scalar>();
    
    template std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<kiss_fft_scalar>& model);
    template std::istream& operator>>(std::istream& is, GaussianMixtureModel<kiss_fft_scalar>& model);
    
}

#include "gmm.hpp"

#include <Eigen/LU>
#include <assert.h>
#include "debug.hpp"

namespace music
{
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(unsigned int dimension) :
        weight(1.0), mean(dimension), preFactor(), rng(new NormalRNG<ScalarType>())
    {
        
    }
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(ScalarType weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean) :
        weight(weight), mean(mean), preFactor(), rng(new NormalRNG<ScalarType>())
    {
        
    }
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(const Gaussian<ScalarType>& other) :
        weight(other.weight), mean(other.mean), preFactor(), rng(new NormalRNG<ScalarType>())
    {
        
    }
    
    template <typename ScalarType>
    ScalarType GaussianDiagCov<ScalarType>::calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist2 = dist;
        ldlt.solve(dist2);
        return preFactor * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    template <typename ScalarType>
    ScalarType GaussianDiagCov<ScalarType>::calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist2 = dist;
        ldlt.solve(dist2);
        return preFactorWithoutWeights * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    template <typename ScalarType>
    ScalarType GaussianDiagCov<ScalarType>::calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> vec2 = dataVector;
        ldlt.solve(vec2);
        return preFactor * std::exp(-0.5 * (dataVector.transpose() * vec2)(0));
    }
    template <typename ScalarType>
    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> GaussianDiagCov<ScalarType>::rand()
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
        preFactor = weight * 1.0/(pow(2*M_PI, diagCov.size()/2.0) * sqrt(diagCov.prod()));
        preFactorWithoutWeights = 1.0/(pow(2*M_PI, diagCov.size()/2.0) * sqrt(diagCov.prod()));
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
    GaussianDiagCov<ScalarType>::GaussianDiagCov(unsigned int dimension) :
        Gaussian<ScalarType>(dimension)
    {
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(ScalarType weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean) :
        Gaussian<ScalarType>(weight, mean), diagCov(mean.size())
    {
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(const GaussianDiagCov<ScalarType>& other) :
        Gaussian<ScalarType>(other), diagCov(other.diagCov)
    {
        calculatePrefactor();
    }
    
    template <typename ScalarType>
    void GaussianFullCov<ScalarType>::calculatePrefactor()
    {
        preFactor = weight * 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
        preFactorWithoutWeights = 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
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
    ScalarType GaussianFullCov<ScalarType>::calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist2 = dist;
        ldlt.solve(dist2);
        return preFactor * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    template <typename ScalarType>
    ScalarType GaussianFullCov<ScalarType>::calculateValueWithoutWeights(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = dataVector - mean;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist2 = dist;
        ldlt.solve(dist2);
        return preFactorWithoutWeights * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    template <typename ScalarType>
    ScalarType GaussianFullCov<ScalarType>::calculateNoMeanValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> vec2 = dataVector;
        ldlt.solve(vec2);
        return preFactor * std::exp(-0.5 * (dataVector.transpose() * vec2)(0));
    }
    template <typename ScalarType>
    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> GaussianFullCov<ScalarType>::rand()
    {
        assert(mean.size() > 0);
        Eigen::LLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > llt(fullCov);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> y(mean.size());
        for (int i=0; i<mean.size(); i++)
            y[i] = rng->rand();
        return llt.matrixL() * y + mean;
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(unsigned int dimension) :
        Gaussian<ScalarType>(dimension), fullCov(dimension, dimension), ldlt()
    {
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(ScalarType weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean) :
        Gaussian<ScalarType>(weight, mean), fullCov(mean.size(), mean.size())
    {
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(const GaussianFullCov<ScalarType>& other) :
        Gaussian<ScalarType>(other), fullCov(other.fullCov)
    {
        calculatePrefactor();
    }
    
    template <typename ScalarType>
    void GaussianMixtureModel<ScalarType>::trainGMM(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount)
    {
        //TODO: precompute good starting vectors.
        gaussians = this->emAlg(std::vector<Gaussian<ScalarType>*>(), data, gaussianCount);
    }
    
    template <typename ScalarType>
    std::vector<Gaussian<ScalarType>*> GaussianMixtureModelFullCov<ScalarType>::emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount, unsigned int maxIterations)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        std::vector<Gaussian<ScalarType>*> gaussians;
        unsigned int dimension = data[0].size();
        assert(dimension > 0);
        unsigned int dataSize = data.size();
        assert(dataSize > 0);
        if (init.empty())
        {
            DEBUG_OUT("no init vectors given. using random values...", 20);
            //init with random data points and identity matricies as covariance matrix
            for (int i=0; i<gaussianCount; i++)
            {
                DEBUG_OUT("adding gaussian distribution " << i << "...", 25);
                Gaussian<ScalarType>* gaussian = new GaussianFullCov<ScalarType>(dimension);
                
                gaussian->setMean(data[std::rand() % dataSize]);
                gaussian->setCovarianceMatrix(10000*Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>::Identity(dimension, dimension));
                gaussians.push_back(gaussian);
            }
        }
        
        //set initial weights all equal to 1/gaussianCount
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> weights = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Constant(gaussianCount, 1.0/ScalarType(gaussianCount));
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> oldWeights = weights;
        Eigen::Array<ScalarType, Eigen::Dynamic, Eigen::Dynamic> p(dataSize, gaussianCount);
        
        
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
            for (int g=0; g<gaussianCount; g++)
            {
                //for every data point do...
                for (unsigned int i=0; i < dataSize; i++)
                {
                    //calculate probability (non-normalized)
                    p(i,g) = gaussians[g]->calculateValueWithoutWeights(data[i]);
                }
            }
            
            //DEBUG_VAR_OUT(p, 0);
            
            ScalarType sum;
            for (unsigned int i=0; i<dataSize; i++)
            {
                //normalize p.
                //sometimes the sum gets 0 or veeeery small, and then
                //dividing by it leads to nan/inf values. we try to
                //skip these values.
                sum = p.row(i).sum();
                if (sum <= 1.0e-280)
                    sum = 1.0;
                
                //resolve aliasing issues here with calling eval()
                p.row(i) = (p.row(i) / sum).eval();
            }
            DEBUG_OUT("E-step END", 30);
            //E-step END
            
            //DEBUG_VAR_OUT(p, 0);
            
            //M-step BEGIN
            DEBUG_OUT("M-step BEGIN", 30);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> prob(gaussianCount);
            for (int g=0; g<gaussianCount; g++)
            {
                //calculate probabilities for a cluster
                prob(g) = p.col(g).mean();
                
                //calculate mu
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mu(dimension);
                mu.setZero();
                for (unsigned int i=0; i<dataSize; i++)
                {
                    //aliasing occurs, but does not harm.
                    mu = mu + (data[i] * p(i,g));
                }
                mu = mu / (dataSize * prob(g));
                
                //calculate sigma
                //TODO: this step needs to be done by the gaussian, in order to reduce calculation costs.
                Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> sigma(dimension, dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = data[i] - mu;
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
            
            if ((oldWeights - weights).norm() / oldWeights.norm() < 10e-6)
                converged = true;
        }
        if (converged)
        {
            DEBUG_OUT("EM converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("EM stopped after " << iteration << " iterations.", 20);
        }
        
        ScalarType sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += weights[g];
            gaussians[g]->setWeight(weights[g]);
            normalizationFactor += gaussians[g]->calculateValue(gaussians[g]->getMean());
        }
        normalizationFactor /= gaussianCount;
        uniRNG = UniformRNG<ScalarType>(0.0, sumOfWeights);
        return gaussians;
    }
    
    template <typename ScalarType>
    std::vector<Gaussian<ScalarType>*> GaussianMixtureModelDiagCov<ScalarType>::emAlg(const std::vector<Gaussian<ScalarType>*>& init, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount, unsigned int maxIterations)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        std::vector<Gaussian<ScalarType>*> gaussians;
        unsigned int dimension = data[0].size();
        assert(dimension > 0);
        unsigned int dataSize = data.size();
        assert(dataSize > 0);
        if (init.empty())
        {
            DEBUG_OUT("no init vectors given. using random values...", 20);
            //init with random data points and identity matricies as covariance matrix
            for (int i=0; i<gaussianCount; i++)
            {
                DEBUG_OUT("adding gaussian distribution " << i << "...", 25);
                Gaussian<ScalarType>* gaussian = new GaussianDiagCov<ScalarType>(dimension);
                
                gaussian->setMean(data[std::rand() % dataSize]);
                gaussian->setCovarianceMatrix(10000*Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>::Identity(dimension, dimension));
                gaussians.push_back(gaussian);
            }
        }
        
        //set initial weights all equal to 1/gaussianCount
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> weights = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Constant(gaussianCount, 1.0/ScalarType(gaussianCount));
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> oldWeights = weights;
        Eigen::Array<ScalarType, Eigen::Dynamic, Eigen::Dynamic> p(dataSize, gaussianCount);
        
        
        unsigned int iteration = 0;
        bool converged = false;
        DEBUG_OUT("starting iteration loop...", 20);
        while ((iteration < maxIterations) && (!converged))
        {
            iteration++;
            oldWeights = weights;
            //assert(weights.sum() > 0.95);
            //assert(weights.sum() < 1.05);
            
            //E-step BEGIN
            DEBUG_OUT("E-step BEGIN", 30);
            //for every gaussian do...
            for (int g=0; g<gaussianCount; g++)
            {
                //for every data point do...
                for (unsigned int i=0; i < dataSize; i++)
                {
                    //calculate probability (non-normalized)
                    p(i,g) = gaussians[g]->calculateValueWithoutWeights(data[i]);
                }
            }
            
            DEBUG_VAR_OUT(p, 50);
            
            ScalarType sum;
            for (unsigned int i=0; i<dataSize; i++)
            {
                //normalize p.
                //sometimes the sum gets 0 or veeeery small, and then
                //dividing by it leads to nan/inf values. we try to
                //skip these values.
                sum = p.row(i).sum();
                if (sum <= 1.0e-280)
                    sum = 1.0;
                
                //resolve aliasing issues here with calling eval()
                p.row(i) = (p.row(i) / sum).eval();
            }
            DEBUG_OUT("E-step END", 30);
            //E-step END
            
            DEBUG_VAR_OUT(p, 50);
            
            //M-step BEGIN
            DEBUG_OUT("M-step BEGIN", 30);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> prob(gaussianCount);
            for (int g=0; g<gaussianCount; g++)
            {
                //calculate probabilities for a cluster
                prob(g) = p.col(g).mean();
                
                //calculate mu
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mu(dimension);
                mu.setZero();
                for (unsigned int i=0; i<dataSize; i++)
                {
                    //aliasing occurs, but does not harm.
                    mu = mu + (data[i] * p(i,g));
                }
                mu = mu / (dataSize * prob(g));
                
                //calculate sigma
                //TODO: this step needs to be done by the gaussian, in order to reduce calculation costs.
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> sigma(dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    //Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> dist = data[i] - mu;
                    sigma = sigma + (p(i,g) * (data[i] - mu).array().square()).matrix();
                }
                sigma = sigma / (dataSize * prob(g));
                
                gaussians[g]->setMean(mu);
                gaussians[g]->setCovarianceMatrix(sigma.asDiagonal());
            }
            DEBUG_OUT("M-step END", 30);
            //M-step END
            weights = prob;
            DEBUG_OUT("check for convergence... weights: " << weights << ", relative change of weights:" << (oldWeights - weights).norm() / oldWeights.norm(), 50);
            
            if ((oldWeights - weights).norm() / oldWeights.norm() < 10e-6)
                converged = true;
        }
        if (converged)
        {
            DEBUG_OUT("EM converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("EM stopped after " << iteration << " iterations.", 20);
        }
        
        ScalarType sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += weights[g];
            gaussians[g]->setWeight(weights[g]);
            normalizationFactor += gaussians[g]->calculateValue(gaussians[g]->getMean());
        }
        normalizationFactor /= gaussianCount;
        uniRNG = UniformRNG<ScalarType>(0.0, sumOfWeights);
        return gaussians;
    }
    
    template <typename ScalarType>
    ScalarType GaussianMixtureModel<ScalarType>::calculateValue(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& pos) const
    {
        ScalarType retVal=0.0;
        for (typename std::vector<Gaussian<ScalarType>*>::const_iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            retVal += (**it).calculateValue(pos);
        }
        return retVal;
    }
    
    template <typename ScalarType>
    ScalarType GaussianMixtureModel<ScalarType>::compareTo(const GaussianMixtureModel<ScalarType>& other)
    {
        //draw some samples from the one model and take a look at the pdf values of the other.
        ScalarType value = 0.0;
        const int numValues = 5000;
        //get the mean of the pdf values. large values should mean "pdfs are similar",
        //and small values indicate that they are not equal.
        for (int i=0; i<numValues; i++)
            value += other.calculateValue(this->rand());
        value /= numValues * normalizationFactor;
        return value;
    }
    
    template <typename ScalarType>
    Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> GaussianMixtureModel<ScalarType>::rand()
    {
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
    template class GaussianMixtureModel<double>;
    template class GaussianMixtureModelFullCov<double>;
    template class GaussianMixtureModelDiagCov<double>;
    
    template std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<double>& model);
    template std::istream& operator>>(std::istream& is, GaussianMixtureModel<double>& model);
    
    template class GaussianFullCov<float>;
    template class GaussianDiagCov<float>;
    template class GaussianMixtureModel<float>;
    template class GaussianMixtureModelFullCov<float>;
    template class GaussianMixtureModelDiagCov<float>;
    
    template std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel<float>& model);
    template std::istream& operator>>(std::istream& is, GaussianMixtureModel<float>& model);
    
    template <typename ScalarType>
    template <int GaussianType>
    void GMM2<ScalarType>::trainGMM(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, int gaussianCount, unsigned int maxIterations)
    {
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> > means;
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> > diagCovs;
        
        unsigned int dimension = data[0].size();
        unsigned int dataSize = data.size();
        
        DEBUG_OUT("no init vectors given. using random values...", 20);
        //init with random data points and identity matricies as covariance matrix
        for (int i=0; i<gaussianCount; i++)
        {
            DEBUG_OUT("adding gaussian distribution " << i << "...", 25);
            
            means.push_back(data[std::rand() % dataSize]);
            diagCovs.push_back(10000*Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>::Identity(dimension, dimension));
        }
        
        //set initial weights all equal to 1/gaussianCount
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> weights = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Constant(gaussianCount, 1.0/ScalarType(gaussianCount));
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> oldWeights = weights;
        Eigen::Array<ScalarType, Eigen::Dynamic, Eigen::Dynamic> p(dataSize, gaussianCount);
        
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
                Eigen::LDLT<Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> > ldlt(diagCovs[g].asDiagonal());
                //diagCovs[g].prod() is equal to its determinant for diagonal matricies.
                double factor = 1.0/(pow(2*M_PI, diagCovs[g].size()/2.0) * sqrt(diagCovs[g].prod()));
                
                //for every data point do...
                for (unsigned int i=0; i < dataSize; i++)
                {
                    //calculate probability (non-normalized)
                    p(i,g) = factor * std::exp(-0.5 * ((data[i] - means[g]).transpose() * ldlt.solve(data[i] - means[g]))(0));
                }
            }
            
            //DEBUG_VAR_OUT(p, 0);
            
            ScalarType sum;
            for (unsigned int i=0; i<dataSize; i++)
            {
                //normalize p.
                //sometimes the sum gets 0 or veeeery small, and then
                //dividing by it leads to nan/inf values. we try to
                //skip these values.
                sum = p.row(i).sum();
                if (sum <= 1.0e-280)
                    sum = 1.0;
                
                //resolve aliasing issues here with calling eval()
                p.row(i) = (p.row(i) / sum).eval();
            }
            DEBUG_OUT("E-step END", 30);
            //E-step END
            
            //DEBUG_VAR_OUT(p, 0);
            
            //M-step BEGIN
            DEBUG_OUT("M-step BEGIN", 30);
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> prob(gaussianCount);
            //calculate probabilities for all clusters
            prob = p.colwise().mean();
            for (int g=0; g<gaussianCount; g++)
            {
                //calculate mu
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mu(dimension);
                mu.setZero();
                for (unsigned int i=0; i<dataSize; i++)
                {
                    //aliasing occurs, but does not harm.
                    mu = mu + (data[i] * p(i,g));
                }
                mu = mu / (dataSize * prob(g));
                
                //calculate sigma
                Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> sigma(dimension);
                sigma.setZero();
                for (unsigned int i=0; i< dataSize; i++)
                {
                    sigma = sigma + (p(i,g) * (data[i] - mu).array().square()).matrix();
                }
                sigma = sigma / (dataSize * prob(g));
                
                means[g] = mu;
                diagCovs[g] = sigma;
            }
            
            DEBUG_OUT("M-step END", 30);
            //M-step END
            weights = prob;
            DEBUG_OUT("check for convergence... weights: " << weights << ", relative change of weights:" << (oldWeights - weights).norm() / oldWeights.norm(), 50);
            if ((oldWeights - weights).norm() / oldWeights.norm() < 10e-6)
                converged = true;
        }
        
        if (converged)
        {
            DEBUG_OUT("EM converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("EM stopped after " << iteration << " iterations.", 20);
        }
        
        for (int g=0; g<gaussianCount; g++)
        {
            DEBUG_VAR_OUT(means[g], 0);
            DEBUG_VAR_OUT(diagCovs[g], 0);
        }
        
        #if 0
        
            
            
            
        
        
        ScalarType sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += weights[g];
            gaussians[g]->setWeight(weights[g]);
            normalizationFactor += gaussians[g]->calculateValue(gaussians[g]->getMean());
        }
        normalizationFactor /= gaussianCount;
        uniRNG = UniformRNG<ScalarType>(0.0, sumOfWeights);
        return gaussians;
        #endif
    }
}

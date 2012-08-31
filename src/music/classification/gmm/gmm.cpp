#include "gmm.hpp"

#include <Eigen/LU>
#include <assert.h>
#include "debug.hpp"

namespace music
{
    Gaussian::Gaussian(unsigned int dimension) :
        weight(1.0), mean(dimension), preFactor(), rng(new NormalRNG<double>())
    {
        
    }
    Gaussian::Gaussian(double weight, Eigen::VectorXd mean) :
        weight(weight), mean(mean), preFactor(), rng(new NormalRNG<double>())
    {
        
    }
    
    
    double GaussianDiagCov::calculateValue(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd dist = dataVector - mean;
        Eigen::VectorXd dist2 = dist;
        ldlt.solve(dist2);
        return preFactor * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    double GaussianDiagCov::calculateValueWithoutWeights(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd dist = dataVector - mean;
        Eigen::VectorXd dist2 = dist;
        ldlt.solve(dist2);
        return preFactorWithoutWeights * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    double GaussianDiagCov::calculateNoMeanValue(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd vec2 = dataVector;
        ldlt.solve(vec2);
        return preFactor * std::exp(-0.5 * (dataVector.transpose() * vec2)(0));
    }
    Eigen::VectorXd GaussianDiagCov::rand()
    {
        assert(mean.size() > 0);
        Eigen::LDLT<Eigen::MatrixXd> ldlt(diagCov.asDiagonal());
        Eigen::VectorXd y(mean.size());
        for (int i=0; i<mean.size(); i++)
            y[i] = rng->rand();
        return ldlt.matrixL() * y + mean;
    }
    void GaussianDiagCov::calculatePrefactor()
    {
        preFactor = weight * 1.0/(pow(2*M_PI, diagCov.size()/2.0) * sqrt(diagCov.prod()));
        preFactorWithoutWeights = 1.0/(pow(2*M_PI, diagCov.size()/2.0) * sqrt(diagCov.prod()));
    }
    void GaussianDiagCov::setCovarianceMatrix(const Eigen::MatrixXd& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        this->diagCov = fullCov.diagonal();
        ldlt.compute(diagCov.asDiagonal());
        
        calculatePrefactor();
    }
    GaussianDiagCov::GaussianDiagCov(unsigned int dimension) :
        Gaussian(dimension)
    {
        
    }
    
    
    void GaussianFullCov::calculatePrefactor()
    {
        preFactor = weight * 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
        preFactorWithoutWeights = 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(fullCov.determinant()));
    }
    void GaussianFullCov::setCovarianceMatrix(const Eigen::MatrixXd& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        this->fullCov = fullCov;
        ldlt.compute(fullCov);
        
        calculatePrefactor();
    }
    double GaussianFullCov::calculateValue(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd dist = dataVector - mean;
        Eigen::VectorXd dist2 = dist;
        ldlt.solve(dist2);
        return preFactor * std::exp(-0.5 * (dist.transpose() * dist2)(0));
    }
    double GaussianFullCov::calculateValueWithoutWeights(const Eigen::VectorXd& dataVector)
    {
        assert(dataVector.size() == mean.size());
        Eigen::VectorXd dist = dataVector - mean;
        Eigen::VectorXd dist2 = dist;
        ldlt.solve(dist2);
        return preFactorWithoutWeights * std::exp(-0.5 * (dist.transpose() * dist2)(0));
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
        assert(mean.size() > 0);
        Eigen::LDLT<Eigen::MatrixXd> ldlt(fullCov);
        Eigen::VectorXd y(mean.size());
        for (int i=0; i<mean.size(); i++)
            y[i] = rng->rand();
        return ldlt.matrixL() * y + mean;
    }
    GaussianFullCov::GaussianFullCov(unsigned int dimension) :
        Gaussian(dimension), fullCov(dimension, dimension), ldlt()
    {
        calculatePrefactor();
    }
    
    void GaussianMixtureModel::trainGMM(std::vector<Eigen::VectorXd> data, int gaussianCount)
    {
        //TODO: precompute good starting vectors.
        gaussians = this->emAlg(std::vector<Gaussian*>(), data, gaussianCount);
    }
    
    std::vector<Gaussian*> GaussianMixtureModel::emAlg(std::vector<Gaussian*> init, std::vector<Eigen::VectorXd> data, int gaussianCount, unsigned int maxIterations)
    {
        //if init is empty, choose some data points as initialization.
        //k-means or something else should be done by somebody else beforehand.
        std::vector<Gaussian*> gaussians;
        unsigned int dimension = data[0].size();
        unsigned int dataSize = data.size();
        if (init.empty())
        {
            DEBUG_OUT("no init vectors given. using random values...", 20);
            //init with random data points and identity matricies as covariance matrix
            for (int i=0; i<gaussianCount; i++)
            {
                DEBUG_OUT("adding gaussian distribution " << i << "...", 25);
                Gaussian* gaussian = new GaussianFullCov(dimension);
                
                gaussian->setMean(data[std::rand() % dataSize]);
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
                    p(i,g) = gaussians[g]->calculateValueWithoutWeights(data[i]);
                }
            }
            
            //DEBUG_VAR_OUT(p, 0);
            
            double sum;
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
                //TODO: this step needs to be done by the gaussian, in order to reduce calculation costs.
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
        
        double sumOfWeights = 0.0;
        normalizationFactor = 0.0;
        for (int g=0; g<gaussianCount; g++)
        {
            sumOfWeights += weights[g];
            gaussians[g]->setWeight(weights[g]);
            normalizationFactor += gaussians[g]->calculateValue(gaussians[g]->getMean());
        }
        normalizationFactor /= gaussianCount;
        uniRNG = UniformRNG<double>(0.0, sumOfWeights);
        return gaussians;
    }
    
    double GaussianMixtureModel::calculateValue(const Eigen::VectorXd& pos) const
    {
        double retVal=0.0;
        for (std::vector<Gaussian*>::const_iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            retVal += (**it).calculateValue(pos);
        }
        return retVal;
    }
    
    double GaussianMixtureModel::compareTo(const GaussianMixtureModel& other)
    {
        //draw some samples from the one model and take a look at the pdf values of the other.
        double value = 0.0;
        const int numValues = 5000;
        //get the mean of the pdf values. large values should mean "pdfs are similar",
        //and small values indicate that they are not equal.
        for (int i=0; i<numValues; i++)
            value += other.calculateValue(this->rand());
        value /= numValues * normalizationFactor;
        return value;
    }
    
    Eigen::VectorXd GaussianMixtureModel::rand()
    {
        double sumOfWeights=0.0;
        double randomNumber = uniRNG.rand();
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
    GaussianMixtureModel::GaussianMixtureModel() :
        gaussians(), uniRNG(0.0, 1.0), normalizationFactor(1.0)
    {
        
    }
    
    std::string GaussianMixtureModel::toJSONString(bool styledWriter) const
    {
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
            Eigen::VectorXd gMean = gaussians[g]->getMean();
            for (int i=0; i<gMean.size(); i++)
                mean.append(Json::Value(gMean[i]));
            arrayElement["mean"]        = mean;
            
            //output the variance as array of double. only save the lower
            //triangular, as the other values are mirrored.
            Json::Value variance(Json::arrayValue);
            Eigen::MatrixXd gVariance = gaussians[g]->getCovarianceMatrix();
            for (int i=0; i<gVariance.rows(); i++)
            {
                for (int j=i; j<gVariance.cols(); j++)
                {
                    variance.append(Json::Value(gVariance(i, j)));
                }
            }
            arrayElement["covariance"]    = variance;
            
            root.append(arrayElement);
        }
        
        std::string str = writer->write(root);
        delete writer;
        return str;
    }
    
    void GaussianMixtureModel::loadFromJSONString(const std::string& jsonString)
    {
        Json::Value root;
        Json::Reader reader;
        reader.parse(jsonString, root, false);
        loadFromJsonValue(root);
    }
    void GaussianMixtureModel::loadFromJsonValue(Json::Value& jsonValue)
    {
        //first: empty list of old gaussians.
        for (unsigned int g=0; g<gaussians.size(); g++)
            delete gaussians[g];
        gaussians.clear();
        
        //read gaussians from json input.
        for (unsigned int g=0; g<jsonValue.size(); g++)
        {
            //first init variables and read dimensionality
            Gaussian* gauss=NULL;
            int dimension=jsonValue[g]["mean"].size();
            bool varianceIsFull;
            if (jsonValue[g]["mean"].size() == jsonValue[g]["covariance"].size())
            {
                gauss = new GaussianDiagCov(dimension);
                varianceIsFull = false;
            }
            else
            {
                gauss = new GaussianFullCov(dimension);
                varianceIsFull = true;
            }
            
            //read weight
            gauss->setWeight(jsonValue[g]["weight"].asDouble());
            
            //read mean
            Eigen::VectorXd mean(dimension);
            Json::Value jMean = jsonValue[g]["mean"];
            for (int i=0; i<dimension; i++)
                mean[i] = jMean[i].asDouble();
            gauss->setMean(mean);
            
            //read covariance
            Eigen::MatrixXd variance(dimension, dimension);
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
    }
    std::ostream& operator<<(std::ostream& os, const GaussianMixtureModel& model)
    {
        //use a fast writer instead of a styled writer, produces smaller JSON
        return (os << model.toJSONString(false));
    }
    std::istream& operator>>(std::istream& is, GaussianMixtureModel& model)
    {
        Json::Value root;
        Json::Reader reader;
        reader.parse(is, root, false);
        model.loadFromJsonValue(root);
        return is;
    }
}

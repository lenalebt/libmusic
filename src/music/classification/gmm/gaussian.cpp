#include "gaussian.hpp"

#include "debug.hpp"
#include "matrixhelper.hpp"

namespace music
{
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(unsigned int dimension, NormalRNG<ScalarType>* rng) :
        weight(1.0), mean(dimension), preFactor(), rng(rng), externalRNG(rng!=NULL), pseudoInverse(NULL)
    {
        mean.setZero();
        
        if (!this->rng)
            this->rng = new NormalRNG<ScalarType>();
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng) :
        weight(weight), mean(mean), preFactor(), rng(rng), externalRNG(rng!=NULL), pseudoInverse(NULL)
    {
        assert(weight >= 0.0);
        assert(weight <= 1.0);
        
        if (!this->rng)
            this->rng = new NormalRNG<ScalarType>();
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>::Gaussian(const Gaussian<ScalarType>& other) :
        weight(other.weight), mean(other.mean), preFactor(), rng(NULL), externalRNG(other.externalRNG), pseudoInverse(NULL)
    {
        if (externalRNG)
            this->rng = other.rng;
        if (!this->rng)
            this->rng = new NormalRNG<ScalarType>();
    }
    
    template <typename ScalarType>
    double Gaussian<ScalarType>::calculateDistance(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector1, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector2)
    {
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> tmp = vector1 - vector2;
        if (pseudoInverse)
        {
            DEBUG_OUT("pseudo", 10);
            DEBUG_VAR_OUT(*pseudoInverse, 0);
            DEBUG_VAR_OUT(tmp, 0);
            DEBUG_VAR_OUT(getCovarianceMatrix(), 0);
            DEBUG_VAR_OUT(getCovarianceMatrix().determinant(), 0);
            DEBUG_VAR_OUT((*pseudoInverse) * tmp, 0);
            DEBUG_VAR_OUT(tmp.transpose() * (*pseudoInverse), 0);
            DEBUG_VAR_OUT(tmp.transpose() * (*pseudoInverse) * tmp, 0);
            return std::sqrt(tmp.transpose() * (*pseudoInverse) * tmp);
        }
        else
        {
            DEBUG_OUT("nonpseudo", 10);
            DEBUG_VAR_OUT(tmp, 0);
            DEBUG_VAR_OUT(getCovarianceMatrix(), 0);
            DEBUG_VAR_OUT(getCovarianceMatrix().determinant(), 0);
            DEBUG_VAR_OUT(tmp.transpose() * llt.solve(tmp), 0);
            return std::sqrt(tmp.transpose() * llt.solve(tmp));
        }
    }
    
    template <typename ScalarType>
    double Gaussian<ScalarType>::calculateDistance(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector1)
    {
        return this->calculateDistance(vector1, this->mean);
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>::~Gaussian()
    {
        if (!externalRNG)
            delete rng;
        if (pseudoInverse)
            delete pseudoInverse;
    }
    
    template <typename ScalarType>
    std::string Gaussian<ScalarType>::toJSONString(bool styledWriter) const
    {
        DEBUG_OUT("saving gaussian as JSON", 40);
        //uses Jsoncpp as library. Jsoncpp is licensed as MIT, so we may use it without restriction.
        Json::Value root;
        Json::Writer* writer=NULL;
        
        if (styledWriter)
            writer = new Json::StyledWriter();
        else
            writer = new Json::FastWriter();
        
        //output the weight
        root["weight"]      = getWeight();
        
        //output the mean as array of doubles
        Json::Value mean(Json::arrayValue);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> gMean = getMean();
        for (int i=0; i<gMean.size(); i++)
            mean.append(Json::Value(gMean[i]));
        root["mean"]        = mean;
        
        //output the variance as array of double. only save the lower
        //triangular, as the other values are mirrored.
        Json::Value variance(Json::arrayValue);
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> gVariance = getCovarianceMatrix();
        if (isDiagonal(gVariance))
        {
            DEBUG_OUT("save as diagonal covariance matrix", 45);
            for (int i=0; i<gVariance.rows(); i++)
            {
                variance.append(Json::Value(gVariance(i, i)));
            }
        }
        else
        {
            DEBUG_OUT("save as full covariance matrix", 45);
            for (int i=0; i<gVariance.rows(); i++)
            {
                for (int j=i; j<gVariance.cols(); j++)
                {
                    variance.append(Json::Value(gVariance(i, j)));
                }
            }
        }
        root["covariance"]    = variance;
        
        std::string str = writer->write(root);
        delete writer;
        return str;
    }
    
    template <typename ScalarType>
    Gaussian<ScalarType>* Gaussian<ScalarType>::loadFromJSONString(const std::string& jsonString)
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
    Gaussian<ScalarType>* Gaussian<ScalarType>::loadFromJsonValue(Json::Value& jsonValue)
    {
        DEBUG_OUT("load gaussian from JSON...", 40);
        
        //first init variables and read dimensionality
        Gaussian<ScalarType>* gauss=NULL;
        int dimension=jsonValue["mean"].size();
        bool varianceIsFull;
        if (jsonValue["mean"].size() == jsonValue["covariance"].size())
        {
            DEBUG_OUT("loading as diagonal covariance matrix", 45);
            gauss = new GaussianDiagCov<ScalarType>(dimension);
            varianceIsFull = false;
        }
        else
        {
            DEBUG_OUT("loading as full covariance matrix", 45);
            gauss = new GaussianFullCov<ScalarType>(dimension);
            varianceIsFull = true;
        }
        
        //read weight
        gauss->setWeight(jsonValue["weight"].asDouble());
        
        //read mean
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean(dimension);
        Json::Value jMean = jsonValue["mean"];
        for (int i=0; i<dimension; i++)
            mean[i] = jMean[i].asDouble();
        gauss->setMean(mean);
        
        //read covariance
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> variance(dimension, dimension);
        Json::Value jVariance = jsonValue["covariance"];
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
        
        return gauss;
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
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> y(mean.size());
        for (int i=0; i<mean.size(); i++)
            y[i] = rng->rand();
        return llt.matrixL() * y + mean;
    }
    template <typename ScalarType>
    void GaussianDiagCov<ScalarType>::calculatePrefactor()
    {
        double determinant = diagCov.cwiseSqrt().prod();
        assert(determinant != 0.0);
        preFactor = weight * 1.0/(pow(2*M_PI, diagCov.size()/2.0) * determinant);
        assert(preFactor != 0.0);
        
        if (determinant < std::numeric_limits<ScalarType>::epsilon()) //not invertible
        {   //calculate moore-penrose pseudoinverse if covariance matrix is not invertible
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> eigenvalues = diagCov;
            Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> invEigenvalues(eigenvalues.size());
            
            for (int i=0; i<eigenvalues.size(); i++)
            {
                if (eigenvalues[i] > eigenvalues.size() * std::numeric_limits<ScalarType>::epsilon())   //sum only nonzero eigenvalues. zero are values smaller than dimension*machineepsilon
                {
                    invEigenvalues[i] = 1.0/eigenvalues[i];
                }
                else
                {
                    invEigenvalues[i] = 0.0;
                }
            }
            
            if (pseudoInverse)
                delete pseudoInverse;
            pseudoInverse = new Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>(invEigenvalues.asDiagonal());
        }
        else
        {
            preFactorWithoutWeights = 1.0/(pow(2*M_PI, diagCov.size()/2.0) * determinant);
            assert(preFactorWithoutWeights != 0.0);
        }
    }
    template <typename ScalarType>
    void GaussianDiagCov<ScalarType>::setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        this->diagCov = fullCov.diagonal();
        ldlt.compute(diagCov.asDiagonal());
        llt.compute(diagCov.asDiagonal());
        
        if (pseudoInverse)
        {
            delete pseudoInverse;
            pseudoInverse = NULL;
        }
        
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(unsigned int dimension, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(dimension, rng)
    {
        assert(this->rng != NULL);
        diagCov.setOnes();
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(weight, mean, rng), diagCov(mean.size())
    {
        assert(this->rng != NULL);
        diagCov.setOnes();
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianDiagCov<ScalarType>::GaussianDiagCov(const GaussianDiagCov<ScalarType>& other) :
        Gaussian<ScalarType>(other), diagCov(other.diagCov)
    {
        assert(this->rng != NULL);
        calculatePrefactor();
        ldlt.compute(diagCov.asDiagonal());
        llt.compute(diagCov.asDiagonal());
    }
    
    template <typename ScalarType>
    void GaussianFullCov<ScalarType>::calculatePrefactor()
    {
        double determinant = fullCov.determinant();
        assert(determinant != 0.0);
        preFactor = weight * 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(determinant));
        
        if (determinant < std::numeric_limits<ScalarType>::epsilon())
        {   //calculate moore-penrose pseudoinverse if covariance matrix is not invertible
            DEBUG_OUT("calculate pseudoinverse", 10);
            Eigen::RealSchur<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> > rSchur(fullCov.template cast<double>());
            Eigen::Matrix<double, Eigen::Dynamic, 1> eigenvalues = rSchur.matrixT().diagonal();
            Eigen::Matrix<double, Eigen::Dynamic, 1> invEigenvalues(eigenvalues.size());
            
            for (int i=0; i<eigenvalues.size(); i++)
            {
                if (eigenvalues[i] > eigenvalues.size() * std::numeric_limits<ScalarType>::epsilon())   //sum only nonzero eigenvalues. zero are values smaller than dimension*machineepsilon
                {
                    DEBUG_OUT("one", 10);
                    DEBUG_VAR_OUT(eigenvalues[i], 0);
                    invEigenvalues[i] = 1.0/eigenvalues[i];
                }
                else
                {
                    DEBUG_OUT("two", 10);
                    DEBUG_VAR_OUT(eigenvalues[i], 0);
                    invEigenvalues[i] = 0.0;
                }
            }
            
            if (pseudoInverse)
                delete pseudoInverse;
            pseudoInverse = new Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>(
                (rSchur.matrixU() * invEigenvalues.asDiagonal() * rSchur.matrixU().transpose()).template cast<ScalarType>()
                );
            DEBUG_VAR_OUT(eigenvalues, 0);
            DEBUG_VAR_OUT(fullCov, 0);
            DEBUG_VAR_OUT(*pseudoInverse, 0);
            DEBUG_VAR_OUT(fullCov * *pseudoInverse, 0);
            DEBUG_VAR_OUT(*pseudoInverse * fullCov, 0);
        }
        else
        {
            assert(preFactor != 0.0);
            preFactorWithoutWeights = 1.0/(pow(2*M_PI, fullCov.rows()/2.0) * sqrt(determinant));
            assert(preFactorWithoutWeights != 0.0);
        }
    }
    template <typename ScalarType>
    void GaussianFullCov<ScalarType>::setCovarianceMatrix(const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& fullCov)
    {
        assert(fullCov.rows() == mean.size());
        this->fullCov = fullCov;
        ldlt.compute(fullCov);
        llt.compute(fullCov);
        
        if (pseudoInverse)
        {
            delete pseudoInverse;
            pseudoInverse = NULL;
        }
        
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
        fullCov = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Ones(dimension).asDiagonal();
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(double weight, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mean, NormalRNG<ScalarType>* rng) :
        Gaussian<ScalarType>(weight, mean, rng), fullCov(mean.size(), mean.size())
    {
        assert(this->rng != NULL);
        fullCov = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>::Ones(mean.size()).asDiagonal();
        calculatePrefactor();
    }
    template <typename ScalarType>
    GaussianFullCov<ScalarType>::GaussianFullCov(const GaussianFullCov<ScalarType>& other) :
        Gaussian<ScalarType>(other), fullCov(other.fullCov)
    {
        assert(this->rng != NULL);
        calculatePrefactor();
        ldlt.compute(fullCov);
        llt.compute(fullCov);
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
    
    
    
    
    template class Gaussian<float>;
    template class Gaussian<double>;
    template class GaussianFullCov<double>;
    template class GaussianDiagCov<double>;
    template class GaussianFullCov<float>;
    template class GaussianDiagCov<float>;
}

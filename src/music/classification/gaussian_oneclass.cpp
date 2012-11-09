#include "gaussian_oneclass.hpp"

namespace music
{
    GaussianOneClassClassifier::GaussianOneClassClassifier() :
        classModel(NULL)
    {
        
    }
    bool GaussianOneClassClassifier::learnModel(const std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& trainingData, ProgressCallbackCaller* callback)
    {
        assert(trainingData.size() > 0);
        int dimension = trainingData[0].size();
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> mean(dimension);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, Eigen::Dynamic> covariance(dimension, dimension);
        
        mean.setZero();
        for (unsigned int i=0; i<trainingData.size(); i++) 
        {
            mean += trainingData[i];
        }
        mean /= trainingData.size();
        
        DEBUG_VAR_OUT(mean, 0);
        
        //this is because we want the variance of chroma and timbre similarity
        //to be centered around 0.0, no matter what the values say. This
        //should be the case by design, so we force it.
        mean[0] = 0.0;
        mean[1] = 0.0;
        
        covariance.setZero();
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmp;
        for (unsigned int i=0; i<trainingData.size(); i++) 
        {
            tmp = trainingData[i]-mean;
            covariance += tmp * tmp.transpose();
        }
        covariance /= trainingData.size();
        
        DEBUG_VAR_OUT(covariance, 0);
        
        //TODO: If covariance matrix is singular (for whatever reason,
        //most probably not enough training samples), do something about
        //it.
        //IDEA: Use Moore-Penrose Pseudoinverse
        //should be integrated in the mahalanobis distance function now
        
        if (classModel != NULL)
            delete classModel;
        classModel = new GaussianFullCov<kiss_fft_scalar>(mean.size());
        classModel->setMean(mean);
        classModel->setCovarianceMatrix(covariance);
        
        return true;
    }
    double GaussianOneClassClassifier::classifyVector(const Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector)
    {
        DEBUG_VAR_OUT(classModel->calculateDistance(vector), 0);
        return classModel->calculateDistance(vector);
    }
}

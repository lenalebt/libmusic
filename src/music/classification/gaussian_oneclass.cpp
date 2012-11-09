#include "gaussian_oneclass.hpp"

namespace music
{
    GaussianOneClassClassifier::GaussianOneClassClassifier() :
        classModel(NULL)
    {
        
    }
    bool GaussianOneClassClassifier::learnModel(const std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& trainingData, ProgressCallbackCaller* callback)
    {
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> mean;
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, Eigen::Dynamic> covariance;
        
        mean.setZero();
        for (unsigned int i=0; i<trainingData.size(); i++) 
        {
            mean += trainingData[i];
        }
        mean /= trainingData.size();
        
        covariance.setZero();
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> tmp;
        for (unsigned int i=0; i<trainingData.size(); i++) 
        {
            tmp = trainingData[i]-mean;
            covariance += tmp.transpose() * tmp;
        }
        covariance /= trainingData.size();
        
        if (classModel != NULL)
            delete classModel;
        classModel = new GaussianFullCov<kiss_fft_scalar>(mean.size());
        classModel->setMean(mean);
        classModel->setCovarianceMatrix(covariance);
        
        return true;
    }
    double GaussianOneClassClassifier::classifyVector(const Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector)
    {
        return classModel->calculateDistance(vector);
    }
}

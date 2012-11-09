#ifndef GAUSSIAN_ONECLASS_HPP
#define GAUSSIAN_ONECLASS_HPP

#include "classifier.hpp"
#include "gmm.hpp"

namespace music
{
    class GaussianOneClassClassifier : public OneClassClassifier<kiss_fft_scalar>
    {
    private:
        GaussianFullCov<kiss_fft_scalar>* classModel;
    protected:
        
    public:
        GaussianOneClassClassifier();
        bool learnModel(const std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector);
        
        GaussianFullCov<kiss_fft_scalar>* getClassModel()        {return classModel;}
    };
    
}

#endif  //GAUSSIAN_ONECLASS_HPP

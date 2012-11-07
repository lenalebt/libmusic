#ifndef GAUSSIAN_ONECLASS_HPP
#define GAUSSIAN_ONECLASS_HPP

#include "classifier.hpp"
#include "gaussian.hpp"

namespace music
{
    class GaussianOneClassClassifier : public OneClassClassifier<kiss_fft_scalar>
    {
    private:
        Gaussian<kiss_fft_scalar>* classModel;
    protected:
        
    public:
        GaussianOneClassClassifier();
        GaussianOneClassClassifier(const GaussianOneClassClassifier& other);
        bool learnModel(const std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector);
        
        std::string toJSONString()                            {return classModel->toJSONString();}
        
        Gaussian<kiss_fft_scalar>* getClassModel()            {return classModel;}
        void setClassModel(Gaussian<kiss_fft_scalar>* model)  {classModel = model->clone();}
    };
    
}

#endif  //GAUSSIAN_ONECLASS_HPP

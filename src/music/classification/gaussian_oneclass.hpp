#ifndef GAUSSIAN_ONECLASS_HPP
#define GAUSSIAN_ONECLASS_HPP

#include "classifier.hpp"

namespace music
{
    template <typename ScalarType = double>
    class GaussianOneClassClassifier : public OneClassClassifier<ScalarType>
    {
    private:
        
    protected:
        
    public:
        bool learnModel(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector);
    };
    
}

#endif  //GAUSSIAN_ONECLASS_HPP

#ifndef LDA_FISHER_HPP
#define LDA_FISHER_HPP

#include "classifier.hpp"

namespace music
{
    class FisherLDAClassifier : public Classifier
    {
    private:
        
    protected:
        Eigen::VectorXd mean1;
        Eigen::VectorXd mean2;
        
        Eigen::MatrixXd covariance1;
        Eigen::MatrixXd covariance2;
    public:
        bool learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::VectorXd& vector);
        
        const Eigen::VectorXd& getClass1Mean() {return mean1;}
        const Eigen::VectorXd& getClass2Mean() {return mean2;}
        const Eigen::MatrixXd& getClass1Covariance() {return covariance1;}
        const Eigen::MatrixXd& getClass2Covariance() {return covariance2;}
    };
}

#endif //LDA_FISHER_HPP

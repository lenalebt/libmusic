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
    public:
        bool learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::VectorXd& vector);
    };
}

#endif //LDA_FISHER_HPP

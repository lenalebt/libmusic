#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP

#include <Eigen/Dense>
#include <utility>
#include <vector>

class Classifier
{
private:
    
protected:
    
public:
    virtual void learnModel()=0;
    virtual void setTrainingData(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData)=0;
    virtual double classifyVector(const Eigen::VectorXd& vector)=0;
};

#endif //CLASSIFIER_HPP

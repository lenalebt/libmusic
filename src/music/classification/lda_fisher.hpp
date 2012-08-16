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
        Eigen::VectorXd mean;
        
        Eigen::MatrixXd covariance1;
        Eigen::MatrixXd covariance2;
        
        Eigen::MatrixXd Sw;
        Eigen::MatrixXd Sb;
        
        Eigen::VectorXd w;      //normal vector of seperating hyperplane
        double w0;              //classification border on projected space
    public:
        bool learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::VectorXd& vector);
        
        const Eigen::VectorXd& getAllClassMean()            {return mean;}
        const Eigen::VectorXd& getClass1Mean()              {return mean1;}
        const Eigen::VectorXd& getClass2Mean()              {return mean2;}
        const Eigen::MatrixXd& getClass1Covariance()        {return covariance1;}
        const Eigen::MatrixXd& getClass2Covariance()        {return covariance2;}
        const Eigen::VectorXd& getHyperplaneNormalVector()  {return w;}
        const Eigen::MatrixXd& getSw()                      {return Sw;}
        const Eigen::MatrixXd& getSb()                      {return Sb;}
    };
}

#endif //LDA_FISHER_HPP

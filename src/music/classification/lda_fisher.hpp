#ifndef LDA_FISHER_HPP
#define LDA_FISHER_HPP

#include "classifier.hpp"

namespace music
{
    /**
     * @brief A Fisher linear disciminant analysis classifier
     * 
     * This class performs a linear discriminant analysis with a Fisher discriminant.
     * It is able to seperate two groups linearly. If it should not be possible to
     * seperate the groups, it might produce weird results.
     * 
     * @remarks Make sure that your data can be linearly seperated before you apply this classifier.
     * @ingroup classification
     * @todo Better documentation (class members are not documented yet)
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    class FisherLDAClassifier : public Classifier
    {
    private:
        bool applyPCA;  //apply PCA before doing LDA? Helps getting Sw invertible.
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
        
        Eigen::MatrixXd reducedU;
        Eigen::MatrixXd doPCA(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData);
    public:
        FisherLDAClassifier(bool applyPCA = false);
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

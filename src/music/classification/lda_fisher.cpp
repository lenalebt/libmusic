#include "lda_fisher.hpp"

namespace music
{
    bool FisherLDAClassifier::learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback)
    {
        double steps = 2.0;
        
        if (callback != NULL)
            callback->progress(0.0, "initializing...");
        //TODO: init
        if (trainingData.size() < 1)
            return true;
        
        if (callback != NULL)
            callback->progress(1.0/steps, "calculating means...");
        
        Eigen::VectorXd tmpVec;
        
        //first: calculate means.
        int class1Count=0, class2Count=0;
        mean1 = Eigen::VectorXd(trainingData[0].first.size());
        mean1.setZero();
        mean2 = Eigen::VectorXd(trainingData[0].first.size());
        mean2.setZero();
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = trainingData.begin(); it != trainingData.end(); it++)
        {
            if (it->second < 0.5)
            {
                class1Count++;
                mean1 += it->first;
            }
            else
            {
                class2Count++;
                mean2 += it->first;
            }
        }
        mean1 /= class1Count;
        mean2 /= class2Count;
        
        if (callback != NULL)
            callback->progress(2.0/steps, "calculating covariance matrices...");
        
        //calculate covariance matricies
        covariance1 = Eigen::MatrixXd(trainingData[0].first.size(), trainingData[0].first.size());
        covariance1.setZero();
        covariance2 = Eigen::MatrixXd(trainingData[0].first.size(), trainingData[0].first.size());
        covariance2.setZero();
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = trainingData.begin(); it != trainingData.end(); it++)
        {
            if (it->second < 0.5)
            {
                //calculate covariance matrix
                tmpVec = it->first - mean1;
                covariance1 += tmpVec * tmpVec.transpose();
            }
            else
            {
                //calculate covariance matrix
                tmpVec = it->first - mean2;
                covariance2 += tmpVec * tmpVec.transpose();
            }
        }
        //divide by number of elements in each class
        covariance1 /= class1Count;
        covariance2 /= class2Count;
        
        
        if (callback != NULL)
            callback->progress(3.0/steps, "calculating optimal discriminant...");
        
        
        if (callback != NULL)
            callback->progress(1.0, "finished.");
        
        
        return true;
    }
    double FisherLDAClassifier::classifyVector(const Eigen::VectorXd& vector)
    {
        return -1.0;
    }
}

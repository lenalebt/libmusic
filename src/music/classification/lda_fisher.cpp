#include "lda_fisher.hpp"

namespace music
{
    bool FisherLDAClassifier::learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback)
    {
        double steps = 5.0;
        
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
        mean = (mean1 + mean2) / (class1Count + class2Count);
        mean1 /= class1Count;
        mean2 /= class2Count;
        
        DEBUG_VAR_OUT(mean1, 0);
        DEBUG_VAR_OUT(mean2, 0);
        
        
        if (callback != NULL)
            callback->progress(2.0/steps, "calculating support vector b...");
        
        //b = (mean1 + mean2) / 2;
        
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
            callback->progress(3.0/steps, "calculating Sw and Sb...");
        //TODO: calculate disriminant
        
        DEBUG_VAR_OUT(covariance1, 0);
        DEBUG_VAR_OUT(covariance2, 0);
        
        Sw = covariance1 + covariance2;
        DEBUG_VAR_OUT(Sw, 0);
        /*{
            Eigen::VectorXd m1 = mean1 - mean;
            Eigen::VectorXd m2 = mean2 - mean;
            
            //DEBUG_VAR_OUT(m1, 0);
            //DEBUG_VAR_OUT(m2, 0);
            
            Sb = m1 * m1.transpose() + m2 * m2.transpose();
            //DEBUG_VAR_OUT(Sb, 0);
        }*/
        
        
        if (callback != NULL)
            callback->progress(4.0/steps, "calculating optimal discriminant...");
        //TODO: calculate disriminant
        //calculate by finding the largest eigenvalue of Sw^-1 * Sb.
        
        w = Sw.inverse() * (mean1 - mean2);
        DEBUG_VAR_OUT(w, 0);
        w0 = w.transpose() * ((mean1 + mean2)/2.0);
        DEBUG_VAR_OUT(w0, 0);
        
        if (callback != NULL)
            callback->progress(1.0, "finished.");
        
        
        return true;
    }
    double FisherLDAClassifier::classifyVector(const Eigen::VectorXd& vector)
    {
        return -(w.transpose() * vector - w0);
    }
}

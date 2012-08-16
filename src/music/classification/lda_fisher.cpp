#include "lda_fisher.hpp"

namespace music
{
    bool FisherLDAClassifier::learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback)
    {
        double steps = 3.0;
        
        if (callback != NULL)
            callback->progress(0.0, "initializing...");
        //TODO: init
        
        if (callback != NULL)
            callback->progress(1.0/steps, "calculating means...");
        
        
        //first: calculate means.
        int class1Count=0, class2Count=0;
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = trainingData.begin(); it != trainingData.end(); it++)
        {
            if (it->second > 0.5)
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
        
        
        return true;
    }
    double FisherLDAClassifier::classifyVector(const Eigen::VectorXd& vector)
    {
        return -1.0;
    }
}

#include "lda_fisher.hpp"

namespace music
{
    Eigen::MatrixXd FisherLDAClassifier::doPCA(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData)
    {
        int vectorSize = trainingData[0].first.size();
        
        //calculate covariance matrix of training data
        Eigen::VectorXd mean(vectorSize);
        mean.setZero();
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = trainingData.begin(); it != trainingData.end(); it++)
        {
            mean += it->first;
        }
        mean /= trainingData.size();
        
        Eigen::MatrixXd covariance(vectorSize, vectorSize);
        covariance.setZero();
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = trainingData.begin(); it != trainingData.end(); it++)
        {
            Eigen::VectorXd tmpVec = it->first - mean;
            covariance += tmpVec * tmpVec.transpose();
        }
        
        //take SVD of covariance matrix
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(covariance, Eigen::ComputeFullU);
        Eigen::MatrixXd u = svd.matrixU();
        
        //Eigen::VectorXd svds = svd.singularValues();
        //DEBUG_VAR_OUT(svds, 0);
        
        //take largest SVDs (which criterion?)
        //TODO: for now, take all nonzero values. might suffice.
        
        //give matrix back that does the transform
        Eigen::MatrixXd reducedU = u.block(0, 0, u.rows(), svd.nonzeroSingularValues());
        
        return reducedU;
    }
    
    FisherLDAClassifier::FisherLDAClassifier(bool applyPCA) :
        applyPCA(applyPCA)
    {
        
    }
    bool FisherLDAClassifier::learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback)
    {
        double steps = 5.0;
        int vectorSize=0;
        
        const std::vector<std::pair<Eigen::VectorXd, double> >* data = NULL;
        
        if (callback != NULL)
            callback->progress(0.0, "initializing...");
        if (applyPCA)
        {
            if (callback != NULL)
                callback->progress(0.25/steps, "applying PCA...");
            reducedU = doPCA(trainingData);
            
            if (callback != NULL)
                callback->progress(0.50/steps, "PCA finished, changing data vectors...");
            
            //calculate new data (apply matrix)
            std::vector<std::pair<Eigen::VectorXd, double> >* newData = new std::vector<std::pair<Eigen::VectorXd, double> >();
            newData->reserve(trainingData.size());
            for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = trainingData.begin(); it != trainingData.end(); it++)
            {
                newData->push_back(std::pair<Eigen::VectorXd, double>(reducedU.transpose() * it->first, it->second));
            }
            data=newData;
            
            if (callback != NULL)
                callback->progress(0.75/steps, "finished changing data vectors! Going on with LDA.");
            //set vectorSize
            vectorSize=(*data)[0].first.size();
        }
        else
        {
            data = &trainingData;
            vectorSize = trainingData[0].first.size();
        }
        //TODO: init
        
        //no data: does not make sense. error.
        if (data->size() < 1)
            return false;
        
        if (callback != NULL)
            callback->progress(1.0/steps, "calculating means...");
        
        Eigen::VectorXd tmpVec;
        
        //first: calculate means.
        int class1Count=0, class2Count=0;
        mean1 = Eigen::VectorXd(vectorSize);
        mean1.setZero();
        mean2 = Eigen::VectorXd(vectorSize);
        mean2.setZero();
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = data->begin(); it != data->end(); it++)
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
        covariance1 = Eigen::MatrixXd(vectorSize, vectorSize);
        covariance1.setZero();
        covariance2 = Eigen::MatrixXd(vectorSize, vectorSize);
        covariance2.setZero();
        for (std::vector<std::pair<Eigen::VectorXd, double> >::const_iterator it = data->begin(); it != data->end(); it++)
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
        w0 = w.transpose() * mean;
        DEBUG_VAR_OUT(w0, 0);
        
        if (applyPCA)
            delete data;
        
        if (callback != NULL)
            callback->progress(1.0, "finished.");
        
        
        return true;
    }
    double FisherLDAClassifier::classifyVector(const Eigen::VectorXd& vector)
    {
        if (applyPCA)
        {
            return -(w.transpose() * (reducedU.transpose() * vector) - w0);
        }
        else
        {
            return -(w.transpose() * vector - w0);
        }
    }
}

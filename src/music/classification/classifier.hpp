#ifndef CLASSIFIER_HPP
#define CLASSIFIER_HPP

#include <Eigen/Dense>
#include <utility>
#include <vector>
#include "progress_callback.hpp"

/**
     * @defgroup classification Classification
     * @brief All classes in this module belong to classification.
     */

/**
 * @brief This is an interface for a data classifier.
 * 
 * This classifier can handle two classes.
 * 
 * @remarks Scaling is done internally!
 * @ingroup classification
 * 
 * @author Lena Brueder
 * @date 2012-08-16
 */
namespace music
{
    class Classifier
    {
    private:
        
    protected:
        
    public:
        virtual bool learnModel(const std::vector<std::pair<Eigen::VectorXd, double> >& trainingData, ProgressCallbackCaller* callback = NULL)=0;
        virtual double classifyVector(const Eigen::VectorXd& vector)=0;
    };
}

#endif //CLASSIFIER_HPP

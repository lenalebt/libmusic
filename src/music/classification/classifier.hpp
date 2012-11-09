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


namespace music
{
    /**
     * @brief This is an interface for an offline data classifier.
     * 
     * This classifier can handle two classes. You need to first call
     * learnModel() to learn a model, and afterwards call
     * classifyVector() on every vector you want to classify.
     * 
     * @tparam ScalarType The type of the scalars in the vectors. Possible values:
     *      <code>double</code> (Standard) and <code>float</code>.
     * 
     * @remarks Scaling is done internally!
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-08-16
     */
    template <typename ScalarType=double>
    class TwoClassClassifier
    {
    private:
        
    protected:
        
    public:
        /**
         * @brief Call this function to learn a model for the data which
         *      will be used to classify given data given through
         *      classifyVector().
         * 
         * This function does offline learning, which means that adding some
         * vectors to refine the classification creates the need for
         * recalculation of the whole model.
         * 
         * @see classifyVector()
         * @return If the learning was successful, or not.
         */
        virtual bool learnModel(const std::vector<std::pair<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>, double> >& trainingData, ProgressCallbackCaller* callback = NULL)=0;
        /**
         * @brief Call this function to get a rating for a vector.
         * 
         * Call the appropriate classification algorithm to classify one vector.
         * 
         * @see learnModel()
         * @return a rating for a vector. Use it to separate the classes.
         */
        virtual double classifyVector(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector)=0;
    };
    
    
    template <typename ScalarType=double>
    class OneClassClassifier
    {
    private:
        
    protected:
        
    public:
        /**
         * @brief Call this function to learn a model for the data which
         *      will be used to classify given data given through
         *      classifyVector().
         * 
         * This function does offline learning, which means that adding some
         * vectors to refine the classification creates the need for
         * recalculation of the whole model.
         * 
         * @see classifyVector()
         * @return If the learning was successful, or not.
         */
        virtual bool learnModel(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& trainingData, ProgressCallbackCaller* callback = NULL)=0;
        /**
         * @brief Call this function to get a rating for a vector.
         * 
         * Call the appropriate classification algorithm to classify one vector.
         * 
         * @see learnModel()
         * @return a rating for a vector. Use it to separate the classes.
         */
        virtual double classifyVector(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector)=0;
    };
}

#endif //CLASSIFIER_HPP

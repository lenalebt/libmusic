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
     * @tparam ScalarType The type of the scalars in the vectors. Possible values:
     *      <code>double</code> (Standard) and <code>float</code>.
     * 
     * @remarks Make sure that your data can be linearly seperated before you apply this classifier.
     * @ingroup classification
     * @todo Better documentation (some class members are not documented yet)
     * 
     * @author Lena Brueder
     * @date 2012-08-27
     */
    template <typename ScalarType=double>
    class FisherLDAClassifier : public TwoClassClassifier<ScalarType>
    {
    private:
        bool applyPCA;  //apply PCA before doing LDA? Helps getting Sw invertible.
    protected:
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean1;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean2;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> mean;
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> covariance1;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> covariance2;
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> Sw;
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> Sb;
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> w;      //normal vector of seperating hyperplane
        double w0;              //classification border on projected space
        
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> reducedU;
        
        /**
         * @brief Performs a PCA (Principal Component Analysis) on the given data.
         * 
         * This algorithm tries to find the "principal components" of the
         * data given. They are defined as the pairwise normal
         * vectors with maximal variance
         * when the data is projected on them. Taking the maximal variance
         * leads to the minimal error.
         * 
         * @return The matrix U, containing the basis vectors of the subspace in which
         *      the data is whitened.
         */
        Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> doPCA(const std::vector<std::pair<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>, double> >& trainingData);
    public:
        /**
         * @brief Creates a new Fisher LDA (Linear Discriminant Analysis) classifier.
         * @param applyPCA Determines if a principal component analysis
         *      should be performed on the data before applying the
         *      Fisher LDA. Sometimes, the results are better with PCA;
         *      but sometimes they are worse. Depends on the problem.
         */
        FisherLDAClassifier(bool applyPCA = false);
        bool learnModel(const std::vector<std::pair<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>, double> >& trainingData, ProgressCallbackCaller* callback = NULL);
        double classifyVector(const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& vector);
        
        /**
         * @brief Return the mean of the data with which the model was formed.
         * @return the mean of the data with which the model was formed.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& getAllClassMean()                  {return mean;}
        /**
         * @brief Return the mean of class 1.
         * @return the mean of class 1.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& getClass1Mean()                    {return mean1;}
        /**
         * @brief Return the mean of class 2.
         * @return the mean of class 2.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& getClass2Mean()                    {return mean2;}
        /**
         * @brief Return the covariance matrix of class 1.
         * @return the covariance matrix of class 1.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& getClass1Covariance() {return covariance1;}
        /**
         * @brief Return the covariance matrix of class 2.
         * @return the covariance matrix of class 2.
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& getClass2Covariance() {return covariance2;}
        /**
         * @brief Return the normal vector of the class-separating hyperplane.
         * @return the normal vector of the class-separating hyperplane
         */
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& getHyperplaneNormalVector()        {return w;}
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& getSw()               {return Sw;}
        const Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>& getSb()               {return Sb;}
    };
}

#endif //LDA_FISHER_HPP

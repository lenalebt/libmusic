#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <Eigen/Dense>
#include <vector>

namespace music
{
    /**
     * @brief This class implements the k-means algorithm.
     * 
     * To start the algorithm, call trainKMeans() with some data. The algorithm
     * performs best if you know the correct number of clusters present
     * in the data.
     * 
     * It is possible to provide the algorithm with some guesses of initial
     * vectors, but you don't have to provide them. If you don't, random
     * data vectors will be chosen. You may also try to find better guesses
     * by feeding the output of calculateInitGuess() into trainKMeans(). Doing so
     * leads to the k-means++-algorithm.
     * 
     * @tparam ScalarType Sets the type of the scalars of the vectors used.
     *      Popular choices may be <code>float</code> and <code>double</code>,
     *      <code>double</code> is selected as a standard value.
     * @tparam AssignmentType Sets the datatype of the internal assignments,
     *      which is used to keep track of the cluster membership of the
     *      data vectors. <code>unsigned char</code> is good for <=256 clusters,
     *      and is selected as standard datatype. If you need more clusters,
     *      change that datatype to an appropriate type. You might need to add
     *      an instantiation line in kmeans.cpp .
     * @ingroup classification
     * 
     * @author Lena Brueder
     * @date 2012-09-06
     */
    template <typename ScalarType=double, typename AssignmentType=unsigned char>
    class KMeans
    {
    private:
        
    protected:
        std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> > means;
        std::vector<AssignmentType> assignments;
    public:
        KMeans();
        /**
         * @brief Runs the k-means algorithm.
         * 
         * k-means is a simple clustering algorithm, which finds clusters
         * in given data. The number of
         * clusters must be known in advance. You can help the algorithm,
         * if you know some good initial guesses for the cluster centers.
         * If you don't know good guesses, it just takes random values
         * out of the data.
         * 
         * @param data The data the algorithm will be run on.
         * @param meanCount The number of clusters, or means.
         * @param maxIterations The maximum number of iterations of the algorithm.
         * @param keepAssignments If the assignments of the data vectors should
         *      be saved when the algorithm finishes, or not. Saves some memory
         *      and a little computation time if selected to not keep them
         *      (time&memory for one copy of <code>data.size()</code> elements
         *      of type <code>unsigned char</code>).
         * @param init Some initial guesses of the cluster centers. Must have
         *      the same number of members as <code>meanCount</code>, otherwise
         *      the guesses will be thrown away.
         * 
         * @return If the algorithm finished by fulfilling the convergence criterion,
         *      or stopped by reaching the maximum number of iterations.
         */
        bool trainKMeans(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int meanCount=10, unsigned int maxIterations=500, bool keepAssignments = false, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& init=std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >());
        
        /**
         * @brief Calculates good initial guesses for the algorithm.
         * 
         * Using these initializations will lead to the k-means++ algorithm.
         * 
         * The initial guesses are calculated such that the space of
         * input data will be represented in a better way than just
         * taking random data.
         * 
         * @param data The data vectors used to find the initial clusters.
         * @param[out] initGuess The initial guesses will be given back in this vector.
         * @param meanCount The number of clusters, or means.
         */
        void calculateInitGuess(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& initGuess, unsigned int meanCount=10);
        
        /**
         * @brief Return the means found by the algorithm.
         * @remarks Will return an empty list if the algorithm has not
         *      been run beforehand.
         * @remarks Returns a const-reference to an internal vector. If you
         *      need a copy, do that for yourself. Keep in mind that this
         *      const-reference changes when you re-run the algorithm!
         * @return the means found by the algorithm.
         */
        const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& getMeans() const      {return means;}
        
        /**
         * @brief Return the cluster assignments of the data vectors to
         *      the clusters found by the algorithm.
         * 
         * The indexes of this vector refer to the indexes in the data
         * vector.
         * 
         * @remarks Will return an empty list if the algorithm has not been
         *      run beforehand, or if <code>keepAssignments</code> in
         *      trainKMeans() was
         *      <code>false</code>.
         * @remarks Returns a const-reference to an internal vector. If you
         *      need a copy, do that for yourself. Keep in mind that this
         *      const-reference changes when you re-run the algorithm!
         * @return the cluster assignments of the data vectors found by the algorithm.
         */
        const std::vector<AssignmentType>& getAssignments() const                               {return assignments;}
    };
}

#endif //KMEANS_HPP

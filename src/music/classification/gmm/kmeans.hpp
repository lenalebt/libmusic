#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <Eigen/Dense>
#include <vector>

namespace music
{
    template <typename T>
    class KMeans
    {
    private:
        
    protected:
        std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> > means;
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
         * @param init Some initial guesses of the cluster centers. Must have
         *      the same number of members as <code>meanCount</code>, otherwise
         *      the guesses will be thrown away.
         * 
         * @return If the algorithm finished by fulfilling the convergence criterion,
         *      or stopped by reaching the maximum number of iterations.
         */
        bool trainKMeans(const std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& data, unsigned int meanCount=10, unsigned int maxIterations=500, const std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& init=std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >());
        
        void getInitGuess(const std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& data, std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& initGuess);
        
        /**
         * @brief Return the means found by the algorithm.
         * @remarks Will return an empty list if the algorithm has not been run beforehand.
         * @return the means found by the algorithm.
         */
        std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> > getMeans() const       {return means;}
    };
}

#endif //KMEANS_HPP

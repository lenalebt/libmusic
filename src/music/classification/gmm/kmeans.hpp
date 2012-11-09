#ifndef KMEANS_HPP
#define KMEANS_HPP

#include <Eigen/Dense>
#include <vector>

namespace music
{
    class KMeans
    {
    private:
        
    protected:
        std::vector<Eigen::VectorXd> means;
    public:
        KMeans();
        void trainKMeans(const std::vector<Eigen::VectorXd>& data, unsigned int meanCount=10, unsigned int maxIterations=500, const std::vector<Eigen::VectorXd>& init=std::vector<Eigen::VectorXd>());
        std::vector<Eigen::VectorXd> getMeans() const       {return means;}
        
        //TODO: return assignments? we don't need them, although it might be useful...
    };
}

#endif //KMEANS_HPP

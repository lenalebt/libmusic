#include "kmeans.hpp"

#include "debug.hpp"
#include <limits>

namespace music
{
    KMeans::KMeans() :
        means()
    {
        
    }
    
    void KMeans::trainKMeans(const std::vector<Eigen::VectorXd>& data, unsigned int meanCount, unsigned int maxIterations, const std::vector<Eigen::VectorXd>& init)
    {
        assert(data.size() > 0u);
        DEBUG_OUT("initialize k-means algorithm...", 20);
        
        means.clear();
        
        unsigned int dataSize = data.size();
        int dimension = data[0].size();
        
        //if no or wrong initialization is given: take random values out of the data
        if (init.empty() || (init.size() != meanCount))
        {
            //TODO: take random values
            DEBUG_OUT("no init vectors given. using random values...", 25);
            //init with random data points and identity matricies as covariance matrix
            for (unsigned int i=0; i<meanCount; i++)
            {
                means.push_back(data[std::rand() % dataSize]);
            }
        }
        
        //initialize assignments
        std::vector<int> assignments(dataSize, -1);
        int assignment;
        double minDistance, distance;
        
        std::vector<unsigned int> vectorCountInCluster(meanCount, 0);
        
        bool converged = false;
        unsigned int iteration = 0;
        DEBUG_OUT("running main loop...", 22);
        while (!converged && (iteration < maxIterations))
        {
            iteration++;
            
            DEBUG_OUT("setting assignments...", 25);
            for (unsigned int i=0; i<dataSize; i++)
            {
                assignment = -1;
                distance = minDistance = std::numeric_limits<double>::max();
                for (unsigned int j=0; j<means.size(); j++)
                {
                    distance = (means[j] - data[i]).norm();
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        assignment = j;
                    }
                }
                assignments[i] = assignment;
            }
            
            DEBUG_OUT("recalculating means...", 25);
            //first set means to zero...
            for (unsigned int i=0; i<meanCount; i++)
            {
                means[i] = Eigen::VectorXd(dimension);
                means[i].setZero();
                vectorCountInCluster[i] = 0;
            }
            //then add up values in each cluster...
            for (unsigned int i=0; i<dataSize; i++)
            {
                means[assignments[i]] += data[i];
                vectorCountInCluster[assignments[i]]++;
            }
            //set mean by dividing through value count in cluster
            for (unsigned int i=0; i<meanCount; i++)
            {
                means[i] = means[i] / vectorCountInCluster[i];
            }
        }
        
        if (converged)
        {
            DEBUG_OUT("k-means converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("k-means stopped after " << iteration << " iterations.", 20);
        }
    }
}

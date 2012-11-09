#include "kmeans.hpp"

#include "debug.hpp"
#include <limits>

namespace music
{
    template<typename T>
    KMeans<T>::KMeans() :
        means()
    {
        
    }
    
    template <typename T>
    bool KMeans<T>::trainKMeans(const std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& data, unsigned int meanCount, unsigned int maxIterations, const std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& init)
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
        unsigned char* assignments = new unsigned char[dataSize];
        unsigned char* oldAssignments = new unsigned char[dataSize];
        unsigned char* tmp = NULL;
        
        unsigned char assignment;
        double minDistance, distance;
        unsigned long assignmentChanges;
        
        std::vector<unsigned int> vectorCountInCluster(meanCount, 0);
        
        bool converged = false;
        unsigned int iteration = 0;
        DEBUG_OUT("running main loop...", 22);
        while (!converged && (iteration < maxIterations))
        {
            iteration++;
            
            //will be set to "false" if convergence is not reached
            //converged = true;
            
            DEBUG_OUT("setting assignments...", 25);
            assignmentChanges = 0;
            for (unsigned int i=0; i<dataSize; i++)
            {
                assignment = -1;
                distance = minDistance = std::numeric_limits<double>::max();
                for (unsigned int j=0; j<means.size(); j++)
                {
                    //since we don't need the distance, but only the /smallest/ distance, we can omit the sqrt operation.
                    //distance = (means[j] - data[i]).norm();
                    distance = (means[j] - data[i]).array().square().sum();
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        assignment = j;
                    }
                }
                assignments[i] = assignment;
                
                //if "false" at one element, will be set to false in the end. if not: no change, convergence.
                //TODO: this criterion does not help when oscillating.
                //converged = converged && (oldAssignments[i] == assignment);
                assignmentChanges += (oldAssignments[i] == assignment) ? 0 : 1;
            }
            if (double(assignmentChanges) / double(dataSize) < 0.002)
                converged = true;
            DEBUG_VAR_OUT(assignmentChanges, 0);
            
            DEBUG_OUT("recalculating means...", 25);
            //first set means to zero...
            for (unsigned int i=0; i<meanCount; i++)
            {
                means[i] = Eigen::Matrix<T, Eigen::Dynamic, 1>(dimension);
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
            
            //for determining convergence
            tmp = oldAssignments;
            oldAssignments = assignments;
            assignments = tmp;
            tmp = NULL;
        }
        
        if (converged)
        {
            DEBUG_OUT("k-means converged after " << iteration << " iterations.", 20);
        }
        else
        {
            DEBUG_OUT("k-means stopped after " << iteration << " iterations.", 20);
        }
        
        delete[] assignments;
        delete[] oldAssignments;
        
        return converged;
    }
    
    template<typename T>
    void KMeans<T>::getInitGuess(const std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& data, std::vector<Eigen::Matrix<T, Eigen::Dynamic, 1> >& initGuess)
    {
        
    }
    
    template class KMeans<double>;
    template class KMeans<float>;
}

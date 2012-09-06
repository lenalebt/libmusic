#include "kmeans.hpp"

#include "debug.hpp"
#include <limits>
#include "randomnumbers.hpp"

namespace music
{
    template<typename ScalarType, typename AssignmentType>
    KMeans<ScalarType, AssignmentType>::KMeans() :
        means(), assignments()
    {
        
    }
    
    template <typename ScalarType, typename AssignmentType>
    bool KMeans<ScalarType, AssignmentType>::trainKMeans(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, unsigned int meanCount, unsigned int maxIterations, bool keepAssignments, const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& init)
    {
        assert(data.size() > 0u);
        assert(meanCount > 0u);
        DEBUG_OUT("initialize k-means algorithm...", 20);
        
        means.clear();
        
        unsigned int dataSize = data.size();
        int dimension = data[0].size();
        
        //if no or wrong initialization is given: take random values out of the data
        if (init.empty() || (init.size() != meanCount))
        {
            DEBUG_OUT("no init vectors given. using random values...", 25);
            for (unsigned int i=0; i<meanCount; i++)
            {
                means.push_back(data[std::rand() % dataSize]);
            }
        }
        
        //initialize assignments
        AssignmentType* assignments = new AssignmentType[dataSize];
        AssignmentType* oldAssignments = new AssignmentType[dataSize];
        AssignmentType* tmp = NULL;
        this->assignments.clear();
        
        AssignmentType assignment;
        double minDistance, distance;
        unsigned long assignmentChanges;
        
        std::vector<unsigned int> vectorCountInCluster(meanCount, 0);
        
        bool converged = false;
        unsigned int iteration = 0;
        DEBUG_OUT("running main loop...", 22);
        while (!converged && (iteration < maxIterations))
        {
            iteration++;
            
            DEBUG_OUT("setting assignments...", 25);
            assignmentChanges = 0;
            for (unsigned int i=0; i<dataSize; i++)
            {
                assignment = -1;
                distance = minDistance = std::numeric_limits<double>::max();
                for (unsigned int j=0; j<means.size(); j++)
                {
                    // since we don't need the distance, but only the
                    // /smallest/ distance, we can omit the sqrt operation.
                    // it is not proven to be faster this way, so I will have
                    // both instructions here - you can choose. both work.
                    distance = (means[j] - data[i]).norm();
                    //distance = (means[j] - data[i]).array().square().sum();
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        assignment = j;
                    }
                }
                assignments[i] = assignment;
                
                //count changes. if no or low change rate, stop.
                assignmentChanges += (oldAssignments[i] == assignment) ? 0 : 1;
            }
            //faster when no jumps are involved. set to converged if <0.2% changed.
            converged = (double(assignmentChanges) / double(dataSize) < 0.002);
            DEBUG_VAR_OUT(assignmentChanges, 0);
            
            DEBUG_OUT("recalculating means...", 25);
            //first set means to zero...
            for (unsigned int i=0; i<meanCount; i++)
            {
                means[i] = Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>(dimension);
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
        
        if (keepAssignments)
        {   //keep assignments to be able to access them later on
            this->assignments.reserve(dataSize);
            for (unsigned int i=0; i<dataSize; i++)
                this->assignments.push_back(assignments[i]);
        }
        
        delete[] assignments;
        delete[] oldAssignments;
        
        return converged;
    }
    
    template<typename ScalarType, typename AssignmentType>
    void KMeans<ScalarType, AssignmentType>::getInitGuess(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& initGuess, unsigned int meanCount)
    {
        assert(meanCount > 0u);
        assert(data.size() > 0u);
        //first clear the initGuess vector. might not be empty.
        initGuess.clear();
        
        UniformRNG<double> rng;
        
        unsigned int dataSize = data.size();
        //insert first point.
        initGuess.push_back(data[std::rand() % dataSize]);
        
        Eigen::VectorXf distances(dataSize);
        
        float distance, minDistance;
        int minDistanceElement;
        double draw;
        
        //create meanCount guesses. first value has been taken.
        for (unsigned int g=1; g<meanCount; g++)
        {
            //calculate distances to nearest cluster mean guess
            for (unsigned int i=0; i<dataSize; i++)
            {
                distance = minDistance = std::numeric_limits<float>::max();
                for (unsigned int j=0; j<initGuess.size(); j++)
                {
                    // since we don't need the distance, but only the
                    // /smallest/ distance, we can omit the sqrt operation.
                    // it is not proven to be faster this way, so I will have
                    // both instructions here - you can choose. both work.
                    distance = (initGuess[j] - data[i]).norm();
                    //distance = (initGuess[j] - data[i]).array().square().sum();
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                    }
                }
                distances[i] = minDistance;
            }
            //draw random value in [0, maxDistance];
            draw = rng.rand() * distances.maxCoeff();
            //find the element that best fits to the drawn value (distance).
            minDistance = std::numeric_limits<float>::max();
            for (unsigned int i=0; i<dataSize; i++)
            {
                distance = fabs(distances[i] - draw);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    minDistanceElement = i;
                }
            }
            //take that element.
            initGuess.push_back(data[minDistanceElement]);
        }
        
        assert(initGuess.size() == meanCount);
    }
    
    template class KMeans<double>;
    template class KMeans<float>;
}

#ifndef TIMBRE_HPP
#define TIMBRE_HPP

#include "constantq.hpp"
#include "dct.hpp"
#include "gmm.hpp"

namespace music
{
    /**
     * @todo Doku schreiben
     * @todo Implementierung überdenken
     * @ingroup feature_extraction
     */
    class TimbreEstimation
    {
    private:
        
    protected:
        
    public:
        
    };

    /**
     * @todo Doku schreiben
     * @todo Implementierung überdenken
     * @ingroup feature_extraction
     */
    class TimbreEstimator
    {
    private:
        
    protected:
        ConstantQTransformResult* transformResult;
        DCT dct;
    public:
        TimbreEstimator(ConstantQTransformResult* transformResult);
        Eigen::VectorXd estimateTimbre(double fromTime, double toTime);
    };
    
    class TimbreModel
    {
    private:
        
    protected:
        ConstantQTransformResult* transformResult;
        GaussianMixtureModel model;
    public:
        TimbreModel(ConstantQTransformResult* transformResult);
        void calculateModel(int modelSize=5, double timeSliceSize=0.02);
        GaussianMixtureModel getModel();
    };
}
#endif  //TIMBRE_HPP

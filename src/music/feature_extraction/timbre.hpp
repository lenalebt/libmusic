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
        GaussianMixtureModel<double>* model;
    public:
        TimbreModel(ConstantQTransformResult* transformResult);
        ~TimbreModel();
        void calculateModel(int modelSize=5, double timeSliceSize=0.02);
        //Model will be property of this class. if you want a copy, go make yourself one!
        GaussianMixtureModel<double>* getModel();
    };
}
#endif  //TIMBRE_HPP

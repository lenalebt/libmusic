#ifndef TIMBRE_HPP
#define TIMBRE_HPP

#include "constantq.hpp"
#include "dct.hpp"

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
}
#endif  //TIMBRE_HPP

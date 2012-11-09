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
        unsigned int timbreVectorSize;
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> cosValues;
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> estimateTimbre1(double fromTime, double toTime);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> estimateTimbre2(double fromTime, double toTime);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> estimateTimbre3(double fromTime, double toTime);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> estimateTimbre4(double fromTime, double toTime);
    public:
        TimbreEstimator(ConstantQTransformResult* transformResult, unsigned int timbreVectorSize=12);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> estimateTimbre(double fromTime, double toTime);
    };
    
    class TimbreModel
    {
    private:
        
    protected:
        ConstantQTransformResult* transformResult;
        GaussianMixtureModel<kiss_fft_scalar>* model;
    public:
        TimbreModel(ConstantQTransformResult* transformResult);
        ~TimbreModel();
        void calculateModel(int modelSize=5, double timeSliceSize=0.02);
        //Model will be property of this class. if you want a copy, go make yourself one!
        GaussianMixtureModel<kiss_fft_scalar>* getModel();
    };
}
#endif  //TIMBRE_HPP

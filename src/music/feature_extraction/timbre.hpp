#ifndef TIMBRE_HPP
#define TIMBRE_HPP

#include "constantq.hpp"
#include "dct.hpp"
#include "gmm.hpp"
#include "progress_callback.hpp"

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
        float minEnergy;
        
    public:
        TimbreEstimator(ConstantQTransformResult* transformResult, unsigned int timbreVectorSize=12, float minEnergy=5.0);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> estimateTimbre(double fromTime, double toTime);
    };
    
    /**
     * @brief Calculates a model for the timbre of a song.
     * 
     * You can set the model size, the time slice size, and the dimensionality of the timbre vectors.
     * 
     * @ingroup feature_extraction
     * 
     * @author Lena Brueder
     * @date 2012-09-20
     */
    class TimbreModel
    {
    private:
        
    protected:
        ConstantQTransformResult* transformResult;
        GaussianMixtureModel<kiss_fft_scalar>* model;
    public:
        /**
         * @brief Creates a timbre model that will be built from this Constant-Q transform result.
         * 
         * Since a call to calculateModel() may take some time, the class
         * will call you back at some events, if you give it a callback.
         * 
         * @see ConstantQTransform
         */
        TimbreModel(ConstantQTransformResult* transformResult);
        ~TimbreModel();
        /**
         * @brief Calculates the model.
         * 
         * This function will start to calculate the timbre vectors and build the model.
         * 
         * @param modelSize The size of the model, e.g. the number of normal distribution
         *      used to model the timbre vectors.
         * @param timeSliceSize The time slice size in seconds that will be used to
         *      create the timbre vectors.
         * @param timbreVectorSize The dimensionality of the timbre vectors (and the resulting model).
         */
        void calculateModel(int modelSize=10, double timeSliceSize=0.02, unsigned int timbreVectorSize=12, ProgressCallbackCaller* callback = NULL);
        /**
         * @brief Calculates the model and preserves the calculated timbre vectors.
         * 
         * This function will start to calculate the timbre vectors and build the model.
         * 
         * @param[in,out] timbreVectors A vector which will contain the timbre vectors. If it is empty,
         *      the timbre vectors will be recalculated. If it is not empty, these vectors will be used.
         *      <code>dimension</code> and the dimension of the data vectors need to agree.
         * @param modelSize The size of the model, e.g. the number of normal distribution
         *      used to model the timbre vectors.
         * @param timeSliceSize The time slice size in seconds that will be used to
         *      create the timbre vectors.
         * @param timbreVectorSize The dimensionality of the timbre vectors (and the resulting model).
         */
        void calculateModel(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& timbreVectors, int modelSize=10, double timeSliceSize=0.02, unsigned int timbreVectorSize=12, ProgressCallbackCaller* callback = NULL);
        /**
         * @brief Return the model that was calculated beforehand.
         * 
         * This function only returns the model that was calculated by
         * calculateModel(). You need to call that function first.
         * 
         * @return A pointer to the generated model. This pointer will be
         *      property of this class. If you need a copy, you need to
         *      clone the object.
         */
        GaussianMixtureModel<kiss_fft_scalar>* getModel();
    };
}
#endif  //TIMBRE_HPP

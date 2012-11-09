#ifndef TIMBRE_HPP
#define TIMBRE_HPP

#include "constantq.hpp"
#include "gmm.hpp"
#include "progress_callback.hpp"

namespace music
{
    /**
     * @brief This class extracts the timbre feature vectors from a ConstantQTransformresult.
     * 
     * The constant Q transform result will be used to calculate a Constant Q Cepstrum,
     * in analogy to the Mel Frequency Cepstrum.
     * 
     * This class uses a discrete cosine transform for decorellation. It does
     * not use a fancy speedup technique, it just precalculates the cosine values
     * since they are very few and the same for every application of the DCT.
     * Since only the first few values of the DCT are needed, it should not be
     * too slow even without speedup techniques which normally are fast if
     * you calculate and need all values at once.
     * 
     * @ingroup feature_extraction
     * 
     * @author Lena Brueder
     * @date 2012-10-18
     */
    class TimbreEstimator
    {
    private:
        
    protected:
        ConstantQTransformResult* transformResult;
        unsigned int timbreVectorSize;
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> cosValues;
        float minEnergy;
        
    public:
        /**
         * @brief Constructs a new TimbreEstimator object which can be
         *      used to estimate the timbre of a song represented through
         *      its Constant Q transform result.
         * 
         * 
         * 
         * @param transformResult A pointer to a ConstantQTransformResult,
         *      which needs to be calculated first. May not be NULL.
         * @param timbreVectorSize The dimension of the timbre vectors produced.
         *      This value affects the quality of the results of the timbre
         *      similarity estimation. Typical values are in the range of 4-40
         *      with a maximum of the number of constant Q bins in the given
         *      <code>transformResult</code>.
         * @param minEnergy The minimal value of the sum of the constant Q transform
         *      result bins in a given time slice that would lead to a valid
         *      timbre vector. This is used because the cepstrum of low-energy signals tends
         *      to be more noisy than the rest of the cepstrum. You can cancel out these
         *      noisy and therefore useless timbre vectors by setting this value
         *      to anything higher than <code>0.0</code>. The higher the value, the
         *      more timbre vectors are canceled out.
         */
        TimbreEstimator(ConstantQTransformResult* transformResult, unsigned int timbreVectorSize=12, float minEnergy=2.0);
        /**
         * @brief Estimates the timbre of the recording in the given time slice.
         * 
         * The algorithm takes the mean of the Constant Q values in the given interval
         * and then calculates a Constant Q Cepstrum, similar to the
         * Mel Frequency Cepstrum.
         * 
         * @param fromTime the beginning of the time slice
         * @param toTime   the end of the time slice
         * 
         * @return A timbre vector of dimension <code>timbreVectorSize</code> if the
         *      sum of the constant Q values in the time slice is higher than
         *      <code>minEnergy</code>, otherwise a vector of dimension 1 with the value
         *      <code>0</code>.
         */
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
         * @brief Calculates the timbre vectors which are needed to calculate the model
         * 
         * @remarks Normally, you will not need this function and instead use calculateModel().
         *      Cases where you might want to use this function is if you want to build a model
         *      directly from some union of timbre vectors.
         * 
         * @param[in,out] timbreVectors A vector which will contain the timbre vectors. The new timbre vectors
         *      will be appended, and it does not matter if <code>timbreVectors</code> is
         *      empty or not. You need to take care that only timbre vectors with the same dimension
         *      will be added to the list.
         * @param timeSliceSize The time slice size in seconds that will be used to
         *      create the timbre vectors.
         * @param timbreVectorSize The dimensionality of the timbre vectors (and the resulting model).
         * 
         * @return if calculating the timbre vectors was successful, or not.
         */
        bool calculateTimbreVectors(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& timbreVectors, double timeSliceSize=0.01, unsigned int timbreVectorSize=12);
        
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
         * 
         * @return if calculating the model was successful, or not.
         */
        bool calculateModel(int modelSize=10, double timeSliceSize=0.01, unsigned int timbreVectorSize=12, ProgressCallbackCaller* callback = NULL);
        
        /**
         * @brief Calculates the model and preserves the calculated timbre vectors.
         * 
         * This function will start to calculate the timbre vectors and build the model.
         * 
         * @param[in,out] timbreVectors A vector which will contain the timbre vectors. If it is empty,
         *      the timbre vectors will be recalculated. If it is not empty, these vectors will be used;
         *      in that case the parameters <code>timeSlizeSize</code> and
         *      <code>timbreVectorSize</code> will be ignored.
         *      <code>dimension</code> and the dimension of the data vectors need to agree.
         * @param modelSize The size of the model, e.g. the number of normal distribution
         *      used to model the timbre vectors.
         * @param timeSliceSize The time slice size in seconds that will be used to
         *      create the timbre vectors.
         * @param timbreVectorSize The dimensionality of the timbre vectors (and the resulting model).
         * 
         * @return if calculating the model was successful, or not.
         */
        bool calculateModel(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& timbreVectors, int modelSize=10, double timeSliceSize=0.02, unsigned int timbreVectorSize=12, ProgressCallbackCaller* callback = NULL);
        
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

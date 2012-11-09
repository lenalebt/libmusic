#ifndef CLASSIFICATION_CATEGORY_HPP
#define CLASSIFICATION_CATEGORY_HPP

#include "gmm.hpp"
#include <Eigen/Dense>
#include "progress_callback.hpp"
#include "databaseentities.hpp"

namespace music
{
    /**
     * @brief This class represents a classification category on application level.
     * 
     * You may use it to ease calculation of scores for recordings.
     * 
     * @ingroup classification
     * @todo documentation
     * @todo implementation
     * 
     * @author Lena Brueder
     * @date 2012-10-12
     */
    class ClassificationCategory
    {
    private:
        GaussianMixtureModel<kiss_fft_scalar>* positiveTimbreModel;
        GaussianMixtureModel<kiss_fft_scalar>* positiveChromaModel;
        GaussianMixtureModel<kiss_fft_scalar>* negativeTimbreModel;
        GaussianMixtureModel<kiss_fft_scalar>* negativeChromaModel;
        
        bool calculateModel(GaussianMixtureModel<kiss_fft_scalar>*& model, std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback = NULL, double initVariance = 100.0, double minVariance = 0.1);
    public:
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> createVectorForFeatures(databaseentities::RecordingFeatures* features, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel);
        
        ClassificationCategory();
        ~ClassificationCategory();
        bool calculatePositiveTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getPositiveTimbreModel() {return positiveTimbreModel;}
        bool calculatePositiveChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getPositiveChromaModel() {return positiveChromaModel;}
        bool calculateNegativeTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getNegativeTimbreModel() {return negativeTimbreModel;}
        bool calculateNegativeChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getNegativeChromaModel() {return negativeChromaModel;}
        
        void setPositiveTimbreModel(GaussianMixtureModel<kiss_fft_scalar>* model)
                    {delete positiveTimbreModel; positiveTimbreModel = model->clone();}
        void setPositiveChromaModel(GaussianMixtureModel<kiss_fft_scalar>* model)
                    {delete positiveChromaModel; positiveChromaModel = model->clone();}
        void setNegativeTimbreModel(GaussianMixtureModel<kiss_fft_scalar>* model)
                    {delete negativeTimbreModel; negativeTimbreModel = model->clone();}
        void setNegativeChromaModel(GaussianMixtureModel<kiss_fft_scalar>* model)
                    {delete negativeChromaModel; negativeChromaModel = model->clone();}
    };
}

#endif //CLASSIFICATION_CATEGORY_HPP

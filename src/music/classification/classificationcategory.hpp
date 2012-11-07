#ifndef CLASSIFICATION_CATEGORY_HPP
#define CLASSIFICATION_CATEGORY_HPP

#include "gmm.hpp"
#include <Eigen/Dense>
#include "progress_callback.hpp"
#include "databaseentities.hpp"
#include "gaussian_oneclass.hpp"

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
        
        bool emptyPositiveTimbreModel;
        bool emptyPositiveChromaModel;
        bool emptyNegativeTimbreModel;
        bool emptyNegativeChromaModel;
        
        GaussianOneClassClassifier* posClassifier;
        GaussianOneClassClassifier* negClassifier;
        
        bool emptyPosClassifierModel;
        bool emptyNegClassifierModel;
        
        bool calculateModel(GaussianMixtureModel<kiss_fft_scalar>*& model, std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback = NULL, double initVariance = 100.0, double minVariance = 0.1);
    public:
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> createVectorForFeatures(databaseentities::RecordingFeatures* features, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel);
        
        ClassificationCategory();
        ~ClassificationCategory();
        bool calculatePositiveTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getPositiveTimbreModel()
        {
            return emptyPositiveTimbreModel ? NULL : positiveTimbreModel;
        }
        void setPositiveTimbreModel(GaussianMixtureModel<kiss_fft_scalar>* model)
        {
            if (model == NULL)
            {
                emptyPositiveTimbreModel = true;
                return;
            }
            delete positiveTimbreModel;
            positiveTimbreModel = model->clone();
        }
        
        bool calculatePositiveChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getPositiveChromaModel()
        {
            return emptyPositiveChromaModel ? NULL : positiveChromaModel;
        }
        void setPositiveChromaModel(GaussianMixtureModel<kiss_fft_scalar>* model)
        {
            if (model == NULL)
            {
                emptyPositiveChromaModel = true;
                return;
            }
            delete positiveChromaModel;
            positiveChromaModel = model->clone();
        }
        
        bool calculateNegativeTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getNegativeTimbreModel()
        {
            return emptyNegativeTimbreModel ? NULL : negativeTimbreModel;
        }
        void setNegativeTimbreModel(GaussianMixtureModel<kiss_fft_scalar>* model)
        {
            if (model == NULL)
            {
                emptyNegativeTimbreModel = true;
                return;
            }
            delete negativeTimbreModel;
            negativeTimbreModel = model->clone();
        }
        
        bool calculateNegativeChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getNegativeChromaModel() 
        {
            return emptyNegativeChromaModel ? NULL : negativeChromaModel;
        }
        void setNegativeChromaModel(GaussianMixtureModel<kiss_fft_scalar>* model)
        {
            if (model == NULL)
            {
                emptyNegativeChromaModel = true;
                return;
            }
            delete negativeChromaModel;
            negativeChromaModel = model->clone();
        }
        
        
        GaussianOneClassClassifier* getPositiveClassifierModel()
        {
            return emptyPosClassifierModel ? NULL : posClassifier;
        }
        GaussianOneClassClassifier* getNegativeClassifierModel()
        {
            return emptyNegClassifierModel ? NULL : negClassifier;
        }
        
        
        
        double classifyRecording(const databaseentities::Recording& recording);
        
        bool calculateClassificatorModel(const std::vector<databaseentities::Recording*>& posExamples, const std::vector<databaseentities::Recording*>& negExamples, unsigned int categoryTimbreModelSize=50, unsigned int categoryTimbrePerSongSampleCount=10000, unsigned int categoryChromaModelSize=8, unsigned int categoryChromaPerSongSampleCount=10000, ProgressCallbackCaller* callback = NULL);
        
        
        
    };
}

#endif //CLASSIFICATION_CATEGORY_HPP

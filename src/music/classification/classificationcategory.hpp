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
     * @brief This class represents a classification category on application
     *      mid level.
     * 
     * You may use it to ease calculation of scores for recordings and to
     * calculate the models for a group of songs. This class represents some
     * mid-level representation. For a high-level view, take a look at
     * ClassificationProcessor.
     * 
     * @ingroup classification
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
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> createVectorForFeatures(databaseentities::RecordingFeatures* features, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> createVectorForFeatures(GaussianMixtureModel<kiss_fft_scalar>* timbreModel, GaussianMixtureModel<kiss_fft_scalar>* chromaModel, double dynamicRange, double length, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel);
        
        /**
         * @brief Calculates a positive timbre model for the given set of
         *      gaussian mixture models (->components)
         * 
         * You need to manually load the models from the database.
         * 
         * @param components The set of gaussian mixture models that will
         *      be used to build a new model out of.
         * @param gaussianCount The number of gaussians in the model that
         *      will be built
         * @param samplesPerGMM The number of samples that will be drawn from
         *      the original set of mixture models
         * @param callback A callback to see the progress of the function.
         *      Will not be used if NULL.
         * 
         * @return if the calculation succeeded.
         */
        bool calculatePositiveTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        /**
         * @brief Calculates a positive chroma model for the given set of
         *      gaussian mixture models (->components)
         * 
         * You need to manually load the models from the database.
         * 
         * @param components The set of gaussian mixture models that will
         *      be used to build a new model out of.
         * @param gaussianCount The number of gaussians in the model that
         *      will be built
         * @param samplesPerGMM The number of samples that will be drawn from
         *      the original set of mixture models
         * @param callback A callback to see the progress of the function.
         *      Will not be used if NULL.
         * 
         * @return if the calculation succeeded.
         */
        bool calculatePositiveChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        /**
         * @brief Calculates a negative timbre model for the given set of
         *      gaussian mixture models (->components)
         * 
         * You need to manually load the models from the database.
         * 
         * @param components The set of gaussian mixture models that will
         *      be used to build a new model out of.
         * @param gaussianCount The number of gaussians in the model that
         *      will be built
         * @param samplesPerGMM The number of samples that will be drawn from
         *      the original set of mixture models
         * @param callback A callback to see the progress of the function.
         *      Will not be used if NULL.
         * 
         * @return if the calculation succeeded.
         */
        bool calculateNegativeTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        /**
         * @brief Calculates a negative chroma model for the given set of
         *      gaussian mixture models (->components)
         * 
         * You need to manually load the models from the database.
         * 
         * @param components The set of gaussian mixture models that will
         *      be used to build a new model out of.
         * @param gaussianCount The number of gaussians in the model that
         *      will be built
         * @param samplesPerGMM The number of samples that will be drawn from
         *      the original set of mixture models
         * @param callback A callback to see the progress of the function.
         *      Will not be used if NULL.
         * 
         * @return if the calculation succeeded.
         */
        bool calculateNegativeChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
    public:
        
        ClassificationCategory();
        ~ClassificationCategory();
        
        
        
        
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
        
        
        /**
         * @brief Gives a score for a recording.
         * 
         * Before using this function, the internal models should be set (via
         * calculateCalssificatorModel or via loadFromJSON).
         * 
         * @param recording A recording that should be classified. The recording
         *      should have its RecordingFeatures loaded.
         * 
         * @return The score of the recording, or 0.0 if no features were found.
         */
        double classifyRecording(const databaseentities::Recording& recording);
        
        /**
         * @brief Calculates the model for the classificator when given a list of
         *      positive and negative examples.
         * 
         * @param negExamples A list of positive examples for the category
         * @param posExamples A list of negative examples for the category
         * @param categoryTimbreModelSize The size of the timbre model (pos&neg) (number of gaussians)
         * @param categoryTimbrePerSongSampleCount The number of samples drawn from the timbre models
         * @param categoryChromaModelSize The size of the chroma model (pos&neg) (number of gaussians)
         * @param categoryChromaPerSongSampleCount The number of samples drawn from the chroma models
         * @param callback A callback to see the progress of the function.
         *      Will not be used if NULL.
         * 
         * @return 
         */
        bool calculateClassificatorModel(const std::vector<databaseentities::Recording*>& posExamples, const std::vector<databaseentities::Recording*>& negExamples, unsigned int categoryTimbreModelSize=50, unsigned int categoryTimbrePerSongSampleCount=10000, unsigned int categoryChromaModelSize=8, unsigned int categoryChromaPerSongSampleCount=2000, ProgressCallbackCaller* callback = NULL);
        
        /**
         * @brief Loads all models from the given JSON strings.
         * 
         * This function sets all internal models from the given JSON strings.
         * It is mainly for convenience, you could load the strings for yourself
         * and set it with the apropriate setter methods. However, this
         * function has some speed advantages (minimal).
         * 
         * @param positiveTimbreModelString The positive timbre model as JSON,
         *       as returned from the database.
         * @param positiveChromaModelString The positive chroma model as JSON,
         *       as returned from the database.
         * @param negativeTimbreModelString The negative timbre model as JSON,
         *       as returned from the database.
         * @param negativeChromaModelString The negative chroma model as JSON,
         *       as returned from the database.
         * @param positiveClassifierDescriptionString The positive classifier
         *       description string as JSON, as returned from the database.
         * @param negativeClassifierDescriptionString The negative classifier
         *       description string as JSON, as returned from the database.
         * 
         * @return 
         */
        void loadFromJSON(const std::string& positiveTimbreModelString, const std::string& positiveChromaModelString, const std::string& negativeTimbreModelString, const std::string& negativeChromaModelString, const std::string& positiveClassifierDescription, const std::string& negativeClassifierDescription);
    };
}

#endif //CLASSIFICATION_CATEGORY_HPP

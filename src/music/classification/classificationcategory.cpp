#include "classificationcategory.hpp"

namespace music
{
    bool ClassificationCategory::calculateModel(GaussianMixtureModel<kiss_fft_scalar>*& model, std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback, double initVariance, double minVariance)
    {
        if (callback)
            callback->progress(0.0, "init...");
        
        if (components.size() == 0)
        {
            return false;
        }
        
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > samples;
        int i=0;
        
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = components.begin(); it != components.end(); it++)
        {
            for (unsigned int j=0; j<samplesPerGMM; j++)
            {
                samples.push_back((*it)->rand());
            }
            
            if (callback)
                callback->progress(0.5 * double(i)/components.size(), "generating samples...");
            i++;
        }
        
        if (callback)
            callback->progress(0.5, "training models...");
        
        //then train the model (best-of-three).
        if (callback)
            callback->progress(0.5,  "calculating model 1");
        model->trainGMM(samples, gaussianCount, initVariance, minVariance);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel1 = model->clone();
        if (callback)
            callback->progress(0.65, "calculating model 2");
        model->trainGMM(samples, gaussianCount, initVariance, minVariance);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel2 = model->clone();
        if (callback)
            callback->progress(0.8,  "calculating model 3");
        model->trainGMM(samples, gaussianCount, initVariance, minVariance);
        GaussianMixtureModel<kiss_fft_scalar>* tmpModel3 = model->clone();
        
        delete model;
        model = NULL;
        
        if (callback)
            callback->progress(0.95, "choose best model");
        
        //choose best-of-three
        if (tmpModel1->getModelLogLikelihood() > tmpModel2->getModelLogLikelihood())
        {
            if (tmpModel1->getModelLogLikelihood() > tmpModel3->getModelLogLikelihood())
            {
                model = tmpModel1;
                delete tmpModel2;
                delete tmpModel3;
            }
            else
            {
                model = tmpModel3;
                delete tmpModel1;
                delete tmpModel2;
            }
        }
        else
        {
            if (tmpModel2->getModelLogLikelihood() > tmpModel3->getModelLogLikelihood())
            {
                model = tmpModel2;
                delete tmpModel1;
                delete tmpModel3;
            }
            else
            {
                model = tmpModel3;
                delete tmpModel1;
                delete tmpModel2;
            }
        }
        
        
        if (callback)
            callback->progress(1.0, "finished!");
        
        return true;
    }
    
    bool ClassificationCategory::calculatePositiveTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return !(emptyPositiveTimbreModel = !calculateModel(positiveTimbreModel, components, gaussianCount, samplesPerGMM, callback));
    }
    
    bool ClassificationCategory::calculatePositiveChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return !(emptyPositiveChromaModel = !calculateModel(positiveChromaModel, components, gaussianCount, samplesPerGMM, callback, 1e-8, 1e-10));
    }
    
    bool ClassificationCategory::calculateNegativeTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return !(emptyNegativeTimbreModel = !calculateModel(negativeTimbreModel, components, gaussianCount, samplesPerGMM, callback));
    }
    
    bool ClassificationCategory::calculateNegativeChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return !(emptyNegativeChromaModel = !calculateModel(negativeChromaModel, components, gaussianCount, samplesPerGMM, callback, 1e-8, 1e-10));
    }

    ClassificationCategory::ClassificationCategory() :
        positiveTimbreModel(new GaussianMixtureModelDiagCov<kiss_fft_scalar>()),
        //positiveChromaModel(new GaussianMixtureModelFullCov<kiss_fft_scalar>()),  //TODO: use this. has errors.
        positiveChromaModel(new GaussianMixtureModelDiagCov<kiss_fft_scalar>()),
        negativeTimbreModel(new GaussianMixtureModelDiagCov<kiss_fft_scalar>()),
        //negativeChromaModel(new GaussianMixtureModelFullCov<kiss_fft_scalar>()),  //TODO: use this. has errors.
        negativeChromaModel(new GaussianMixtureModelDiagCov<kiss_fft_scalar>()),
        emptyPositiveTimbreModel(true),
        emptyPositiveChromaModel(true),
        emptyNegativeTimbreModel(true),
        emptyNegativeChromaModel(true),
        posClassifier(new GaussianOneClassClassifier()),
        negClassifier(new GaussianOneClassClassifier()),
        emptyPosClassifierModel(true),
        emptyNegClassifierModel(true)
    {
        
    }
    ClassificationCategory::~ClassificationCategory()
    {
        if (positiveTimbreModel)
            delete positiveTimbreModel;
        if (positiveChromaModel)
            delete positiveChromaModel;
        if (negativeTimbreModel)
            delete negativeTimbreModel;
        if (negativeChromaModel)
            delete negativeChromaModel;
        if (posClassifier)
            delete posClassifier;
        if (negClassifier)
            delete negClassifier;
    }
    
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> ClassificationCategory::createVectorForFeatures(databaseentities::RecordingFeatures* features, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel)
    {
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec(4);
        
        GaussianMixtureModel<kiss_fft_scalar>* timbreModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(features->getTimbreModel());
        GaussianMixtureModel<kiss_fft_scalar>* chromaModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(features->getChromaModel());
        
        vec = createVectorForFeatures(timbreModel, chromaModel, features->getDynamicRange(), features->getLength(), categoryTimbreModel, categoryChromaModel);
        
        delete timbreModel;
        delete chromaModel;
        
        return vec;
    }
    
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> ClassificationCategory::createVectorForFeatures(GaussianMixtureModel<kiss_fft_scalar>* timbreModel, GaussianMixtureModel<kiss_fft_scalar>* chromaModel, double dynamicRange, double length, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel)
    {
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec(4);
        if (categoryTimbreModel)
            vec[0] = timbreModel->compareTo(*categoryTimbreModel);
        else
            vec[0] = 0.0;
        
        if (categoryChromaModel)
            vec[1] = chromaModel->compareTo(*categoryChromaModel);
        else
            vec[1] = 0.0;
        vec[2] = dynamicRange;
        vec[3] = length;
        
        return vec;
    }
    
    
    bool ClassificationCategory::calculateClassificatorModel(const std::vector<databaseentities::Recording*>& posExamples, const std::vector<databaseentities::Recording*>& negExamples, unsigned int categoryTimbreModelSize, unsigned int categoryTimbrePerSongSampleCount, unsigned int categoryChromaModelSize, unsigned int categoryChromaPerSongSampleCount, ProgressCallbackCaller* callback)
    {
        if (callback)
            callback->progress(0.0, "loading timbre and chroma models...");
        
        //for positives: load timbre & chroma models
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> positiveTimbreModels;
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> positiveChromaModels;
        for (std::vector<databaseentities::Recording*>::const_iterator it = posExamples.begin(); it != posExamples.end(); ++it)
        {
            GaussianMixtureModel<kiss_fft_scalar>* gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString((*it)->getRecordingFeatures()->getTimbreModel());
            positiveTimbreModels.push_back(gmm);
            gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString((*it)->getRecordingFeatures()->getChromaModel());
            positiveChromaModels.push_back(gmm);
        }
        //for negatives: load timbre & chroma models
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> negativeTimbreModels;
        std::vector<GaussianMixtureModel<kiss_fft_scalar>*> negativeChromaModels;
        for (std::vector<databaseentities::Recording*>::const_iterator it = negExamples.begin(); it != negExamples.end(); ++it)
        {
            GaussianMixtureModel<kiss_fft_scalar>* gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString((*it)->getRecordingFeatures()->getTimbreModel());
            negativeTimbreModels.push_back(gmm);
            gmm = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString((*it)->getRecordingFeatures()->getChromaModel());
            negativeChromaModels.push_back(gmm);
        }
        
        if (callback)
            callback->progress(0.1, "processing timbre models...");
        
        //combine positive timbre models
        if (positiveTimbreModels.size() != 0)
        {
            if (!calculatePositiveTimbreModel(positiveTimbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount, callback))
                return false;
        }
        else
            emptyPositiveTimbreModel = true;
        
        //combine negative timbre models
        if (negativeTimbreModels.size() != 0)
        {
            if (!calculateNegativeTimbreModel(negativeTimbreModels, categoryTimbreModelSize, categoryTimbrePerSongSampleCount, callback))
                return false;
        }
        else
            emptyNegativeTimbreModel = true;
        
        if (callback)
            callback->progress(0.5, "processing chroma models...");
        
        //combine positive chroma models
        if (positiveChromaModels.size() != 0)
        {
            if (!calculatePositiveChromaModel(positiveChromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount, callback))
                return false;
        }
        else
            emptyPositiveChromaModel = true;
        
        //combine negative chroma models
        if (negativeChromaModels.size() != 0)
        {
            if (!calculateNegativeChromaModel(negativeChromaModels, categoryChromaModelSize, categoryChromaPerSongSampleCount, callback))
                return false;
        }
        else
            emptyNegativeChromaModel = true;
        
        //we do not need the pos/neg chroma and timbre anymore. save space, delete them!
        //delete timbreModels
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = positiveTimbreModels.begin(); it != positiveTimbreModels.end(); ++it)
        {
            delete *it;
        }
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = negativeTimbreModels.begin(); it != negativeTimbreModels.end(); ++it)
        {
            delete *it;
        }
        //delete chromaModels
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = positiveChromaModels.begin(); it != positiveChromaModels.end(); ++it)
        {
            delete *it;
        }
        for (std::vector<GaussianMixtureModel<kiss_fft_scalar>*>::iterator it = negativeChromaModels.begin(); it != negativeChromaModels.end(); ++it)
        {
            delete *it;
        }
        positiveTimbreModels.clear();
        positiveChromaModels.clear();
        negativeTimbreModels.clear();
        negativeChromaModels.clear();
        
        //up to here: combined the timbres of category example positives to a new model for the category.
        //now: build model of category when timbre and chroma was applied.
        //first: create vectors
        
        if (callback)
            callback->progress(0.8, "processing classifier models...");
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec;
        
        DEBUG_VAR_OUT(positiveChromaModel->toJSONString(), 0);
        
        //positive
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > positiveExampleVectors;
        for (std::vector<databaseentities::Recording*>::const_iterator it = posExamples.begin(); it != posExamples.end(); ++it)
        {
            vec = createVectorForFeatures((*it)->getRecordingFeatures(), positiveTimbreModel, positiveChromaModel);
            positiveExampleVectors.push_back(vec);
        }
        
        //negative
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > negativeExampleVectors;
        for (std::vector<databaseentities::Recording*>::const_iterator it = negExamples.begin(); it != negExamples.end(); ++it)
        {
            vec = createVectorForFeatures((*it)->getRecordingFeatures(), negativeTimbreModel, negativeChromaModel);
            negativeExampleVectors.push_back(vec);
        }
        
        //build models for classifiers
        if (positiveExampleVectors.size() > 0)
        {
            emptyPosClassifierModel = false;
            posClassifier->learnModel(positiveExampleVectors, callback);
        }
        else
            emptyPosClassifierModel = true;
        
        if (negativeExampleVectors.size() > 0)
        {
            emptyNegClassifierModel = false;
            negClassifier->learnModel(negativeExampleVectors, callback);
        }
        else
            emptyNegClassifierModel = true;
        
        if (callback)
            callback->progress(1.0, "finished");
        
        return true;
    }
    
    void ClassificationCategory::loadFromJSON(const std::string& positiveTimbreModelString, const std::string& positiveChromaModelString, const std::string& negativeTimbreModelString, const std::string& negativeChromaModelString, const std::string& positiveClassifierDescriptionString, const std::string& negativeClassifierDescriptionString)
    {
        if (!(emptyPositiveTimbreModel = positiveTimbreModelString.empty()))
            positiveTimbreModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(positiveTimbreModelString);
        if (!(emptyPositiveChromaModel = positiveChromaModelString.empty()))
            positiveChromaModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(positiveChromaModelString);
        if (!(emptyNegativeTimbreModel = negativeTimbreModelString.empty()))
            negativeTimbreModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(negativeTimbreModelString);
        if (!(emptyNegativeChromaModel = negativeChromaModelString.empty()))
            negativeChromaModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(negativeChromaModelString);
        if (!(emptyPosClassifierModel = positiveClassifierDescriptionString.empty()))
            posClassifier->loadFromJSONString(positiveClassifierDescriptionString);
        if (!(emptyNegClassifierModel = negativeClassifierDescriptionString.empty()))
            negClassifier->loadFromJSONString(negativeClassifierDescriptionString);
    }
    
    double ClassificationCategory::classifyRecording(const databaseentities::Recording& recording)
    {
        GaussianMixtureModel<kiss_fft_scalar>* timbreModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(recording.getRecordingFeatures()->getTimbreModel());
        GaussianMixtureModel<kiss_fft_scalar>* chromaModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(recording.getRecordingFeatures()->getChromaModel());
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> posVec = createVectorForFeatures(timbreModel, chromaModel, recording.getRecordingFeatures()->getDynamicRange(), recording.getRecordingFeatures()->getLength(), emptyPositiveTimbreModel ? NULL : positiveTimbreModel, emptyPositiveChromaModel ? NULL : positiveChromaModel);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> negVec = createVectorForFeatures(timbreModel, chromaModel, recording.getRecordingFeatures()->getDynamicRange(), recording.getRecordingFeatures()->getLength(), emptyNegativeTimbreModel ? NULL : negativeTimbreModel, emptyNegativeChromaModel ? NULL : negativeChromaModel);
        
        delete timbreModel;
        delete chromaModel;
                
        DEBUG_VAR_OUT(emptyPositiveTimbreModel, 0);
        DEBUG_VAR_OUT(emptyPositiveChromaModel, 0);
        DEBUG_VAR_OUT(emptyNegativeTimbreModel, 0);
        DEBUG_VAR_OUT(emptyNegativeChromaModel, 0);
        DEBUG_VAR_OUT(emptyPosClassifierModel , 0);
        DEBUG_VAR_OUT(emptyNegClassifierModel , 0);
        
        
        
        return (emptyPosClassifierModel ? 0.0 : posClassifier->classifyVector(posVec))
            + (emptyNegClassifierModel ? 0.0 : negClassifier->classifyVector(negVec));
    }
}

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
        
        std::cerr << tmpModel1->toJSONString() << std::endl;
        std::cerr << tmpModel2->toJSONString() << std::endl;
        std::cerr << tmpModel3->toJSONString() << std::endl;
        
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
        return calculateModel(positiveTimbreModel, components, gaussianCount, samplesPerGMM, callback);
    }
    
    bool ClassificationCategory::calculatePositiveChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return calculateModel(positiveChromaModel, components, gaussianCount, samplesPerGMM, callback, 1e-8, 1e-10);
    }
    
    bool ClassificationCategory::calculateNegativeTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return calculateModel(negativeTimbreModel, components, gaussianCount, samplesPerGMM, callback);
    }
    
    bool ClassificationCategory::calculateNegativeChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        return calculateModel(negativeChromaModel, components, gaussianCount, samplesPerGMM, callback, 1e-8, 1e-10);
    }

    ClassificationCategory::ClassificationCategory() :
        positiveTimbreModel(new GaussianMixtureModelDiagCov<kiss_fft_scalar>()),
        positiveChromaModel(new GaussianMixtureModelFullCov<kiss_fft_scalar>()),
        negativeTimbreModel(new GaussianMixtureModelDiagCov<kiss_fft_scalar>()),
        negativeChromaModel(new GaussianMixtureModelFullCov<kiss_fft_scalar>())
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
    }
    
    Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> ClassificationCategory::createVectorForFeatures(databaseentities::RecordingFeatures* features, GaussianMixtureModel<kiss_fft_scalar>* categoryTimbreModel, GaussianMixtureModel<kiss_fft_scalar>* categoryChromaModel)
    {
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> vec(4);
        
        GaussianMixtureModel<kiss_fft_scalar>* timbreModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(features->getTimbreModel());
        GaussianMixtureModel<kiss_fft_scalar>* chromaModel = GaussianMixtureModel<kiss_fft_scalar>::loadFromJSONString(features->getChromaModel());
        
        if (categoryTimbreModel)
            vec[0] = timbreModel->compareTo(*categoryTimbreModel);
        else
            vec[0] = 0.0;
        
        if (categoryChromaModel)
            vec[1] = chromaModel->compareTo(*categoryChromaModel);
        else
            vec[1] = 0.0;
        vec[2] = features->getDynamicRange();
        vec[3] = features->getLength();
        
        return vec;
    }
}

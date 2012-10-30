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
            callback->progress(0.5, "training model...");
        
        if (model)
            delete model;
        model = new GaussianMixtureModelDiagCov<kiss_fft_scalar>();
        model->trainGMM(samples, gaussianCount, initVariance, minVariance);
        
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
        positiveTimbreModel(NULL),
        positiveChromaModel(NULL),
        negativeTimbreModel(NULL),
        negativeChromaModel(NULL)
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
}

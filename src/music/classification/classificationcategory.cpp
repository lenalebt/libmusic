#include "classificationcategory.hpp"

namespace music
{
    bool ClassificationCategory::calculateTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback)
    {
        if (callback)
            callback->progress(0.0, "init...");
        
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
        model->trainGMM(samples, gaussianCount);
        
        if (callback)
            callback->progress(1.0, "finished!");
        
        return true;
    }

    ClassificationCategory::ClassificationCategory() :
        model(NULL)
    {
        
    }
    ClassificationCategory::~ClassificationCategory()
    {
        if (model)
            delete model;
    }
}

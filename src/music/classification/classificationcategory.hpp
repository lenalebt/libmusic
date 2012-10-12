#ifndef CLASSIFICATION_CATEGORY_HPP
#define CLASSIFICATION_CATEGORY_HPP

#include "gmm.hpp"
#include <Eigen/Dense>
#include "progress_callback.hpp"

namespace music
{
    //TODO: add callbacks
    class ClassificationCategory
    {
    private:
        GaussianMixtureModel<kiss_fft_scalar>* model;
    public:
        ClassificationCategory();
        ~ClassificationCategory();
        bool calculateTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getTimbreModel() {return model;}
    };
}

#endif //CLASSIFICATION_CATEGORY_HPP

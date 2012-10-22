#ifndef CLASSIFICATION_CATEGORY_HPP
#define CLASSIFICATION_CATEGORY_HPP

#include "gmm.hpp"
#include <Eigen/Dense>
#include "progress_callback.hpp"

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
        GaussianMixtureModel<kiss_fft_scalar>* timbreModel;
        GaussianMixtureModel<kiss_fft_scalar>* chromaModel;
        
        bool calculateModel(GaussianMixtureModel<kiss_fft_scalar>*& model, std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount, unsigned int samplesPerGMM, ProgressCallbackCaller* callback = NULL, double initVariance = 100.0, double minVariance = 0.1);
    public:
        ClassificationCategory();
        ~ClassificationCategory();
        bool calculateTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getTimbreModel() {return timbreModel;}
        bool calculateChromaModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 8, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getChromaModel() {return chromaModel;}
    };
}

#endif //CLASSIFICATION_CATEGORY_HPP

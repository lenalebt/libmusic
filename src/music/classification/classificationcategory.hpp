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
    public:
        ClassificationCategory();
        ~ClassificationCategory();
        bool calculateTimbreModel(std::vector<GaussianMixtureModel<kiss_fft_scalar>*> components, unsigned int gaussianCount = 50, unsigned int samplesPerGMM = 10000, ProgressCallbackCaller* callback = NULL);
        GaussianMixtureModel<kiss_fft_scalar>* getTimbreModel() {return timbreModel;}
    };
}

#endif //CLASSIFICATION_CATEGORY_HPP

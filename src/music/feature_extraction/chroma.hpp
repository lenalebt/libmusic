#ifndef CHROMA_HPP
#define CHROMA_HPP

#include <string>
#include "constantq.hpp"
#include <ostream>
#include "gmm.hpp"
#include "progress_callback.hpp"

#include <Eigen/Dense>

namespace music
{
    /**
     * @ingroup feature_extraction
     */
    class ChromaEstimator
    {
    private:
        ConstantQTransformResult* transformResult;
        double timeResolution;
        
        void applyNonlinearFunction(Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1>& vector);
        static std::string getNoteName(int i);
    protected:
    public:
        ChromaEstimator(ConstantQTransformResult* transformResult);
        bool estimateChroma(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& chromaVectors, int& mode, double timeSliceLength = 0.01, bool makeTransposeInvariant = true);
    };
    
    /**
     * @ingroup feature_extraction
     */
    class ChromaModel
    {
    private:
        ConstantQTransformResult* transformResult;
        GaussianMixtureModel<kiss_fft_scalar>* model;
    protected:
        int mode;
    public:
        ChromaModel(ConstantQTransformResult* transformResult);
        ~ChromaModel();
        
        bool calculateChromaVectors(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& chromaVectors, double timeSliceLength=0.01, bool makeTransposeInvariant = true);
        
        bool calculateModel(unsigned int modelSize=10, double timeSliceLength=0.01, bool makeTransposeInvariant = true, ProgressCallbackCaller* callback = NULL);
        
        //bool calculateModel(std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> >& chromaVectors, int modelSize=10, double timeSliceLength=0.02, unsigned int timbreVectorSize=12, ProgressCallbackCaller* callback = NULL);
        
        GaussianMixtureModel<kiss_fft_scalar>* getModel();
        int getMode();
    };
}
#endif  //CHROMA_HPP

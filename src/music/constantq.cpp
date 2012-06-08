#include "constantq.hpp"

//use for filtering
#include <musicaccess.hpp>

#include <assert.h>

namespace music
{
    ConstantQTransform::ConstantQTransform() :
        octaveCount(8),
        fMin(40),
        fMax(11025),
        fs(22050),
        binsPerOctave(12),
        lowpassFilter(NULL),
        q(1.0),
        threshold(0.0005),
        nkMax(0),
        fKernel(NULL)
    {
        
    }
    
    ConstantQTransform* ConstantQTransform::createTransform(musicaccess::IIRFilter* lowpassFilter, int binsPerOctave, int fMin, int fMax, int fs, double q, double threshold, double atomHopFactor)
    {
        assert(lowpassFilter != NULL);
        
        ConstantQTransform* cqt = NULL;
        cqt = new ConstantQTransform();
        assert(cqt != NULL);
        
        //set internal variables
        double fRatio = fMax/fMin;
        cqt->octaveCount = std::ceil(log2(fRatio));
        //recalculate the minimum frequency from the number of octaves involved such that we always calculate full octaves
        cqt->fMin = fMax / std::pow(2.0, cqt->octaveCount) * std::pow(2.0, 1.0/binsPerOctave);
        cqt->fMax = fMax;
        cqt->fs = fs;
        cqt->binsPerOctave = binsPerOctave;
        cqt->lowpassFilter = lowpassFilter;
        cqt->q = q;
        cqt->threshold = threshold;
        cqt->atomHopFactor = atomHopFactor;
        //this is what they use in the matlab implementation. Where went deltaOmega from eq.5 in the paper?
        cqt->Q = q/(std::pow(2.0, 1.0/binsPerOctave) - 1);
        //calculate the length of the largest atom in samples
        cqt->nkMax = Q * double(fs) / double(fMin) + 0.5;   //+0.5 for rounding
        
        //calculate the length of the shortest atom in samples
        int nkMin = Q * double(fs) / (double(fMin) * std::pow(2.0, double(binsPerOctave-1)/double(binsPerOctave)) + 0.5;    //+0.5 for rounding
        int atomHop = nkMin * atomHopFactor + 0.5;
        
        //TODO: Calculate spectral kernels for one octave
        DynamicSparseMatrix tmpFKernel;     //Eigens DynamicSparseMatrix can be filled in an easier way than its SparseMatrix.
        //take a look at line 60 in genCQTkernel.m
        
        
        return cqt;
    }
    
    void ConstantQTransform::apply(uint16_t* buffer, int sampleCount)
    {
        assert(this->lowpassFilter != NULL);
        assert(this->fKernel != NULL);
        //TODO: Map buffer to a Eigen vector via Map<>, see
        // http://eigen.tuxfamily.org/dox/TutorialMapClass.html
        
        //padding with zeros in the beginning and in the end
        
        //then run over samples and calculate cqt.
        //in the end, we will get a matrix with roughly (binsPerOctave*octaveCount)x(sampleCount/timesliceLength) entries.
        //that matrix will have more entries for higher frequencies, and lesser for lower frequencies.
    }
}

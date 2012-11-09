#include "constantq.hpp"

//use for filtering
#include "musicaccess.hpp"

namespace music
{
    ConstantQTransform* ConstantQTransform::createTransform(int binsPerOctave, int fMin, int fMax, int fs, float q, float threshold, musicaccess::IIRFilter* lowpassFilter)
    {
        return NULL;
    }
    
    void apply(uint16_t* buffer, int sampleCount)
    {
        //TODO: Map buffer to a Eigen vector via Map<>, see
        // http://eigen.tuxfamily.org/dox/TutorialMapClass.html
    }
}

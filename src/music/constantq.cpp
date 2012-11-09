#include "constantq.hpp"

//use for filtering
#include "musicaccess.hpp"

namespace music
{
    ConstantQTransform* ConstantQTransform::createTransform(int binsPerOctave, int fMin, int fMax, int fs, float q, float threshold, musicaccess::IIRFilter* lowpassFilter)
    {
        return NULL;
    }
}

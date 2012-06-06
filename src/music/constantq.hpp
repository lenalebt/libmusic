#ifndef CONSTANT_Q_HPP
#define CONSTANT_Q_HPP

namespace music
{

    class ConstantQTransform
    {
    private:
        ConstantQTransform();
    public:
        //todo: add parameters
        ConstantQTransform* createTransform(int bins, int fMin, int fMax);
        //todo: replace void with proper rückgabewert
        void apply(uint16_t* buffer, int sampleCount);
    };
}
#endif //CONSTANT_Q_HPP

#ifndef EMAUDIO_FILTER_HPP
#define EMAUDIO_FILTER_HPP

class EMAudioFilter
{
public:
    EMAudioFilter();

    // --- Audio Filter Interface Methods --- //
    virtual void filter(float* dest, float* input, size_t buffersLen) = 0;

    virtual ~EMAudioFilter();
};

#endif // EMAUDIO_FILTER_HPP
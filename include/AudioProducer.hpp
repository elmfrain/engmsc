#ifndef EMAUDIO_PRODUCER_HPP
#define EMAUDIO_PRODUCER_HPP

class EMAudioProducer
{
public:
    EMAudioProducer();

    // --- Audio Producer Interface Methods --- //

    // Add generated samples into buffer
    virtual size_t placeSamples(float* buffer, size_t bufferLen, float gain = 1.0f) = 0;
    virtual double getDuration() const = 0;
    virtual bool hasExpired() const = 0;

    virtual ~EMAudioProducer();
};

#endif // EMAUDIO_PRODUCER_HPP
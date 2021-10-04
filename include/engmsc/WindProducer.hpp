#ifndef WIND_PRODUCER_HPP
#define WIND_PRODUCER_HPP

#include <engmsc/IAudioProducer.hpp>
#include <iir/Butterworth.h>

class WindProducer : public IAudioProducer
{
public:
    virtual size_t produceSamples(float* buffer, size_t bufferSize) override;
    virtual size_t addOntoSamples(float* buffer, size_t bufferSize, float gain = 1.0f) override;
    virtual double getDuration() const override;
    virtual bool hasExpired() const override;

    void setWindVelocity(double windVelocity);
    void expire();
private:
    bool m_expired = false;
    Iir::Butterworth::LowPass<4> m_lowPass;
    double m_windVelocity = 0.0;
};

#endif
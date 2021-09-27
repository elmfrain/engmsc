#pragma once

#ifndef KICK_PRODUCER_HPP
#define KICK_PRODUCER_HPP

#include <engmsc/IAudioProducer.hpp>

class KickProducer : public IAudioProducer
{
public:
    KickProducer(float factor = 1.0f, float factor2 = 1.0f);

    virtual size_t produceSamples(float* buffer, size_t bufferSize);
    virtual size_t addOntoSamples(float* buffer, size_t bufferSize, float gain = 1.0f);
    virtual double getDuration() const;
    virtual bool hasExpired() const;
private:
    float m_factor;
    float m_factor2;
    bool m_hasExpired = false;
    size_t m_samplePos = 0;
    inline float genSample() const;
};

#endif
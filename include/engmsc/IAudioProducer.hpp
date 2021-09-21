#pragma once

#ifndef I_AUDIO_PRODUCER_HPP
#define I_AUDIO_PRODUCER_HPP

#include <inttypes.h>
#include <stddef.h>

class IAudioProducer
{
public:
    IAudioProducer();

    virtual size_t produceSamples(float* buffer, size_t bufferLen) = 0;
    virtual size_t addOntoSamples(float* buffer, size_t bufferLen, float gain = 1.0f) = 0;
    virtual double getDuration() const = 0;
    virtual bool hasExpired() const = 0;

    virtual ~IAudioProducer();
};

#endif
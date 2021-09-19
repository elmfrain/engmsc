#pragma once

#ifndef I_AUDIO_PRODUCER_HPP
#define I_AUDIO_PRODUCER_HPP

#include <inttypes.h>
#include <stddef.h>

class IAudioProducer
{
public:
    virtual size_t produceSamples(uint16_t* buffer, size_t bufferLen) = 0;
    virtual double getDuration() = 0;
};

#endif
#pragma once

#ifndef SOUND_EVENT_HPP
#define SOUND_EVENT_HPP

#include <engmsc/IAudioProducer.hpp>

struct SoundEvent
{
public:
    SoundEvent(IAudioProducer* producer, float volume = 1.0f, float pitch = 1.0f);

    float volume = 1.0f;
    float pitch = 1.0f;
    IAudioProducer* audioProducer = nullptr;
private:
    friend class AudioStream;
    double m_samplePos = 0.0;
};

#endif
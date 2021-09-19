#pragma once

#ifndef I_AUDIO_CONTEXT_HPP
#define I AUDIO_CONTEXT_HPP

#include <engmsc/AudioStream.hpp>

class IAudioContext
{
public:
    IAudioContext();

    virtual bool initContext(void* userData = nullptr) = 0;
    virtual void addStream(AudioStream& audioStream) = 0;
    virtual bool removeStream(AudioStream& audioStream) = 0;
    virtual void destroyContext() = 0;

    IAudioContext(const IAudioContext& copy) = delete;
    void operator=(const IAudioContext& copy) = delete;
};

#endif
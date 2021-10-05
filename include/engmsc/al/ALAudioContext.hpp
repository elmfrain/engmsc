#pragma once

#ifndef AL_AUDIO_CONTEXT_HPP
#define AL_AUDIO_CONTEXT_HPP

#include <engmsc/IAudioContext.hpp>

#ifdef _WIN32
    #include <alc.h>
    #include <al.h>
#elif __linux__
    #include <AL/alc.h>
    #include <AL/al.h>
#elif __APPLE__
    #include <OpenAL/alc.h>
    #include <OpenAL/al.h>
#endif

#include <forward_list>

class ALAudioContext : public IAudioContext
{
public:
    virtual bool initContext(void* userData = nullptr);
    virtual void addStream(AudioStream& audioStream);
    virtual bool removeStream(AudioStream& audioStream);
    virtual void destroyContext();
private:
    ALCdevice* m_alDevice = nullptr;
    ALCcontext* m_alContext = nullptr;

    struct StreamChannel
    {
        AudioStream* audioStream = nullptr;
        ALuint alSource = 0;
        ALuint alBufferPool[BUFFER_POOL_SIZE];
        size_t underruns = 0;
    };

    std::mutex m_streamListMutex;
    std::forward_list<StreamChannel> m_activeStreams;

    std::mutex m_workerMutex;
    std::condition_variable m_workerCV;
    std::thread* m_workerThread;
    bool m_workerRunning = true;
    void i_streamWorkerThread();
};

#endif
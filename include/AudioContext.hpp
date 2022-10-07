#ifndef EMAUDIO_CONTEXT_HPP
#define EMAUDIO_CONTEXT_HPP

#include "AudioStream.hpp"

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

// Audio Context Interface
class EMAudioContext
{
public:
    EMAudioContext();
    
    // Audio Context Interface Methods
    virtual bool initContext(void* userData = NULL) = 0;
    virtual void addStream(EMAudioStream& audioStream) = 0;
    virtual bool removeStream(EMAudioStream& audioStream) = 0;
    virtual void destoryContext() = 0;

    EMAudioContext(const EMAudioContext& copy) = delete;
};

class EMOpenALContext : public EMAudioContext
{
public:
    virtual bool initContext(void* userData = NULL) override;
    virtual void addStream(EMAudioStream& audioStream) override;
    virtual bool removeStream(EMAudioStream& audioStream) override;
    virtual void destoryContext() override;
    
    void update();
private:
    ALCdevice* m_alDevice = NULL;
    ALCcontext* m_alContext = NULL;

    struct StreamChannel
    {
        EMAudioStream* stream;
        ALuint alSource = 0;
        ALuint alBufferPool[EMBUFFER_POOL_SIZE];
        int underruns = 0;
    };

    std::mutex m_streamMutex;
    std::forward_list<StreamChannel> m_streams;
};

#endif // EMAUDIO_CONTEXT_HPP
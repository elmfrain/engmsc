#include "AudioContext.hpp"

#include "GLFWInclude.hpp"
#include "Logger.hpp"

EMAudioContext::EMAudioContext()
{

}

// -------- EMOpenALContext Class -------- //

static EMLogger m_logger("OpenAL");

static const int m_bufferSizeBytes = EMSAMPLES_PER_BUFFER * sizeof(EMAudioStream::data);
static const double m_bufferDuration = double(EMSAMPLES_PER_BUFFER) / EMSAMPLE_RATE;

bool EMOpenALContext::initContext(void* userData)
{
    m_alDevice = alcOpenDevice(NULL);
    if(!m_alDevice)
    {
        m_logger.errorf("Failed to find an audio device");
        return false;
    }

    m_alContext = alcCreateContext(m_alDevice, NULL);
    if(!m_alContext)
    {
        m_logger.errorf("Failed to create context");
        return false;
    }

    m_logger.infof("Successfully initialized context");
    return true;
}

void EMOpenALContext::addStream(EMAudioStream& audioStream)
{
    alcMakeContextCurrent(m_alContext);

    StreamChannel channel;
    channel.stream = & audioStream;
    alGenSources(1, &channel.alSource);
    alGenBuffers(EMBUFFER_POOL_SIZE, channel.alBufferPool);

    for(ALuint buffer : channel.alBufferPool)
    {
        alBufferData(buffer, AL_FORMAT_MONO16, audioStream.getNextBuffer(), m_bufferSizeBytes, EMSAMPLE_RATE);
    }

    alSourceQueueBuffers(channel.alSource, EMBUFFER_POOL_SIZE, channel.alBufferPool);
    alSourcePlay(channel.alSource);
    audioStream.restart();

    std::unique_lock<std::mutex> lock(m_streamMutex);
    m_streams.push_front(channel);
}

bool EMOpenALContext::removeStream(EMAudioStream& audioStream)
{
    alcMakeContextCurrent(m_alContext);

    EMAudioStream* streamPtr = &audioStream;

    std::unique_lock<std::mutex> lock(m_streamMutex);
    for(StreamChannel& channel : m_streams)
    {
        if(channel.stream == streamPtr)
        {
            alDeleteSources(1, &channel.alSource);
            alDeleteBuffers(EMBUFFER_POOL_SIZE, channel.alBufferPool);

            return true;
        }
    }

    return false;
}

void EMOpenALContext::destoryContext()
{
    for(StreamChannel& channel : m_streams)
    {
        alDeleteSources(1, &channel.alSource);
        alDeleteBuffers(EMBUFFER_POOL_SIZE, channel.alBufferPool);
    }

    alcDestroyContext(m_alContext);
    alcCloseDevice(m_alDevice);
}

void EMOpenALContext::update()
{
    std::unique_lock<std::mutex> lock(m_streamMutex);
    for(StreamChannel& channel : m_streams)
    {
        int buffersProcessed = 0;
        alGetSourcei(channel.alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);

        if(buffersProcessed <= 0) continue;

        while(buffersProcessed--)
        {
            ALuint buffer;
            alSourceUnqueueBuffers(channel.alSource, 1, &buffer);

            alBufferData(buffer, AL_FORMAT_MONO16, channel.stream->getNextBuffer(), m_bufferSizeBytes, EMSAMPLE_RATE);

            alSourceQueueBuffers(channel.alSource, 1, &buffer);
        }

        int state = 0;
        alGetSourcei(channel.alSource, AL_SOURCE_STATE, &state);
        if(state == AL_STOPPED)
        {
            alSourcePlay(channel.alSource);
            channel.underruns++;
        }
    }
}
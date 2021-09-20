#include <engmsc/al/ALAudioContext.hpp>

#include <iostream>
#include <cstring>

typedef std::chrono::high_resolution_clock MainClock;

static const int BUFFER_SIZE_BYTES = SAMPLES_PER_BUFFER * sizeof(int16_t);

static const double BUFFER_DURATION = double(SAMPLES_PER_BUFFER) / SAMPLE_RATE;

static const MainClock::duration WORKER_INTERVAL = std::chrono::duration_cast<MainClock::duration>(
    std::chrono::duration<double>(BUFFER_DURATION / 2.0)
);

bool ALAudioContext::initContext(void* userData)
{
    m_alDevice = alcOpenDevice(nullptr);
    if(!m_alDevice)
    {
        std::cerr << "[ALAudioContext : Error]: Failed to find an audio device!" << std::endl;
        return false;
    }

    m_alContext = alcCreateContext(m_alDevice, nullptr);
    if(!m_alContext)
    {
        std::cerr << "[ALAudioContext : Error]: Failed to initialize AL context!" << std::endl;
        return false;
    }

    m_workerThread = new std::thread(&ALAudioContext::i_streamWorkerThread, this);

    return true;
}

void ALAudioContext::addStream(AudioStream& audioStream)
{
    alcMakeContextCurrent(m_alContext);

    StreamChannel streamChannel;
    streamChannel.audioStream = &audioStream;
    alGenSources(1, &streamChannel.alSource);
    alGenBuffers(BUFFER_POOL_SIZE, streamChannel.alBufferPool);

    int16_t* blankBuffer = new int16_t[SAMPLES_PER_BUFFER];
    memset(blankBuffer, 0, BUFFER_SIZE_BYTES);
    for(ALuint buffer : streamChannel.alBufferPool)
    {
        alBufferData(buffer, AL_FORMAT_MONO16, blankBuffer, BUFFER_SIZE_BYTES, SAMPLE_RATE);
    }
    alSourceQueueBuffers(streamChannel.alSource, BUFFER_POOL_SIZE, streamChannel.alBufferPool);
    alSourcePlay(streamChannel.alSource);
    delete[] blankBuffer;

    std::unique_lock<std::mutex> lock(m_streamListMutex);
    m_activeStreams.push_front(streamChannel);
}

bool ALAudioContext::removeStream(AudioStream& audioStream)
{
    alcMakeContextCurrent(m_alContext);

    AudioStream* streamPtr = &audioStream;
    bool successfullyRemoved = false;

    std::unique_lock<std::mutex> lock(m_streamListMutex);
    m_activeStreams.remove_if([&](StreamChannel& streamChannel)
    {
        if(streamChannel.audioStream == streamPtr)
        {
            alDeleteSources(1, &streamChannel.alSource);
            alDeleteBuffers(BUFFER_POOL_SIZE, streamChannel.alBufferPool);

            successfullyRemoved = true;
            return true;
        }
        return false;
    });

    return successfullyRemoved;
}

void ALAudioContext::destroyContext()
{
    alcDestroyContext(m_alContext);
    alcCloseDevice(m_alDevice);

    m_workerRunning = false;
    m_workerCV.notify_one();
    m_workerThread->join();
    delete m_workerThread;
}

void ALAudioContext::i_streamWorkerThread()
{
    while(m_workerRunning)
    {
        {
            alcMakeContextCurrent(m_alContext);

            std::unique_lock<std::mutex>lock(m_streamListMutex);
            for(StreamChannel& streamChannel : m_activeStreams)
            {
                int buffersProcessed = 0;
                alGetSourcei(streamChannel.alSource, AL_BUFFERS_PROCESSED, &buffersProcessed);

                if(buffersProcessed <= 0 || !streamChannel.audioStream->hasNextReady()) continue;

                while(buffersProcessed--)
                {
                    ALuint buffer;
                    alSourceUnqueueBuffers(streamChannel.alSource, 1, &buffer);

                    alBufferData(buffer, AL_FORMAT_MONO16, streamChannel.audioStream->getNextBuffer(), BUFFER_SIZE_BYTES, SAMPLE_RATE);

                    alSourceQueueBuffers(streamChannel.alSource, 1, &buffer);
                }

                int sourceState = 0;
                alGetSourcei(streamChannel.alSource, AL_SOURCE_STATE, &sourceState);
                if(sourceState == AL_STOPPED) 
                {
                    alSourcePlay(streamChannel.alSource);
                    std::cout << ++streamChannel.underruns << std::endl;
                }
            }
        }

        std::unique_lock<std::mutex> lock(m_workerMutex);
        m_workerCV.wait_for(lock, WORKER_INTERVAL);
    }
}
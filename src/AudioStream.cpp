#include <engmsc/AudioStream.hpp>
#include <chrono>
#include <math.h>

typedef std::chrono::high_resolution_clock MainClock;

static const double BUFFER_DURATION = double(SAMPLES_PER_BUFFER) / SAMPLE_RATE;

static const MainClock::duration WORKER_INTERVAL = std::chrono::duration_cast<MainClock::duration>(
    std::chrono::duration<double>(BUFFER_DURATION)
);

AudioStream::AudioStream() :
    m_bufferPoolData(new uint16_t[SAMPLES_PER_BUFFER * BUFFER_POOL_SIZE]),
    m_timeStreamStarted(MainClock::now().time_since_epoch().count())
{
    for(int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        m_bufferPool[i].id = i;
        m_bufferPool[i].data = m_bufferPoolData + SAMPLES_PER_BUFFER * i;
        m_inputBufferQueue.push(&m_bufferPool[i]);
    }

    m_bufferingThread = new std::thread(&AudioStream::i_bufferingThread, this);
}

void AudioStream::playEvent(const SoundEvent& soundEvent)
{
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, BUFFER_DURATION + getTime());
}

void AudioStream::playEventAt(const SoundEvent& soundEvent, double seconds)
{
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, BUFFER_DURATION + seconds);
}

void AudioStream::playEventIn(const SoundEvent& soundEvent, double seconds)
{
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, BUFFER_DURATION + getTime() + seconds);
}

const int16_t* AudioStream::getNextBuffer()
{
    std::unique_lock<std::mutex> lock(m_bufferQueueMutex);
    const uint16_t* nextData = m_outputBufferQueue.front()->data;
    m_inputBufferQueue.push(m_outputBufferQueue.front());
    m_outputBufferQueue.pop();
    m_bufferingThreadCV.notify_one();
    return (const int16_t*) nextData;
}

bool AudioStream::hasNextReady()
{
    m_bufferingThreadCV.notify_one();
    return !m_outputBufferQueue.empty();
}

double AudioStream::getTime()
{
    size_t now = MainClock::now().time_since_epoch().count();
    return (now - m_timeStreamStarted) / 10e8;
}

AudioStream::~AudioStream()
{
    delete[] m_bufferPoolData;
    m_exitThread = true;
    m_bufferingThreadCV.notify_one();
    m_bufferingThread->join();
    delete m_bufferingThread;
}

AudioStream::TimedSoundEvent::TimedSoundEvent(const SoundEvent& p_event, double p_time) :
    event(p_event),
    timeToPlay(p_time) {}

void AudioStream::i_bufferingThread()
{
    #define F(x) (1000.0 / (10 * double(x) / SAMPLE_RATE + 1))
    #define G(x) (1.0 / (10.0 * double(x) / SAMPLE_RATE + 1))
    int j = 1;
    while(!m_exitThread)
    {
        {
            std::unique_lock<std::mutex> lock0(m_bufferQueueMutex);
            while(m_inputBufferQueue.size() > 0)
            {
                Buffer& currentBuffer = *m_inputBufferQueue.front();

                for(int i = 0; i < SAMPLES_PER_BUFFER; i++)
                {
                    float noise = 2.0f * float(rand()) / RAND_MAX - 1.0f;
                    currentBuffer.data[i] = sin((j++ * 3.1415 * (80 + F(j * 10))) / 44100) * 30000 * G(j * 6);
                    currentBuffer.data[i] += noise * 5000 * G(j * 200);

                    if(double(j) / SAMPLE_RATE > 0.5 / getTime())
                    {
                        j = 1;
                    }
                }

                m_outputBufferQueue.push(&currentBuffer);
                m_inputBufferQueue.pop();
            }
        }
        
        std::unique_lock<std::mutex> lock1(m_threadMutex);
        m_bufferingThreadCV.wait_for(lock1, WORKER_INTERVAL);
    }
}
#include <engmsc/AudioStream.hpp>
#include <chrono>
#include <math.h>

typedef std::chrono::high_resolution_clock MainClock;

static const double BUFFER_DURATION = double(SAMPLES_PER_BUFFER) / SAMPLE_RATE;

static const MainClock::duration WORKER_INTERVAL = std::chrono::duration_cast<MainClock::duration>(
    std::chrono::duration<double>(BUFFER_DURATION / 4.0)
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
    m_nbSounds++;
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, BUFFER_DURATION * BUFFER_POOL_SIZE + getTime());
}

void AudioStream::playEventAt(const SoundEvent& soundEvent, double seconds)
{
    m_nbSounds++;
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, BUFFER_DURATION * BUFFER_POOL_SIZE + seconds);
}

void AudioStream::playEventIn(const SoundEvent& soundEvent, double seconds)
{
    m_nbSounds++;
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, BUFFER_DURATION * BUFFER_POOL_SIZE + getTime() + seconds);
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

size_t AudioStream::getNbSounds() const
{
    return m_nbSounds;
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

#include <cstring>

void AudioStream::i_bufferingThread()
{
    double bufferTime = -BUFFER_DURATION * 0.1;
    float* workBuffer = new float[SAMPLES_PER_BUFFER];
    size_t sampleCount = 0;

    while(!m_exitThread)
    {
        {
            std::unique_lock<std::mutex> lock0(m_bufferQueueMutex);
            while(m_inputBufferQueue.size() > 0)
            {
                Buffer& currentBuffer = *m_inputBufferQueue.front();

                memset(workBuffer, 0, SAMPLES_PER_BUFFER * sizeof(float));

                {
                    std::unique_lock<std::mutex> lock1(m_soundsMutex);

                    for(TimedSoundEvent& sound : m_activeSounds)
                    {
                        if(!sound.hasStarted && bufferTime < sound.timeToPlay && sound.timeToPlay < bufferTime + BUFFER_DURATION)
                        {
                            sound.hasStarted = true;
                            int sampleStart = (sound.timeToPlay - bufferTime) * SAMPLE_RATE;
                            sound.event.audioProducer->addOntoSamples(workBuffer + sampleStart, SAMPLES_PER_BUFFER - sampleStart, sound.event.volume);
                            continue;
                        }

                        if(sound.hasStarted)
                        {
                            sound.event.audioProducer->addOntoSamples(workBuffer, SAMPLES_PER_BUFFER, sound.event.volume);
                        }
                    }
    
                    m_activeSounds.remove_if([&](TimedSoundEvent& e)
                    {
                        if(e.event.audioProducer->hasExpired())
                        {
                            m_nbSounds--;
                            delete e.event.audioProducer;
                            return true;
                        }
                        return false;
                    });
                }

                for(int i = 0; i < SAMPLES_PER_BUFFER; i++)
                {
                    currentBuffer.data[i] = workBuffer[i] * 32767;
                }

                m_outputBufferQueue.push(&currentBuffer);
                m_inputBufferQueue.pop();
                bufferTime += BUFFER_DURATION;
            }
        }
        
        std::unique_lock<std::mutex> lock2(m_threadMutex);
        m_bufferingThreadCV.wait_for(lock2, WORKER_INTERVAL);
    }

    delete[] workBuffer;
}
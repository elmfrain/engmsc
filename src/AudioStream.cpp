#include <engmsc/AudioStream.hpp>
#include <chrono>
#include <math.h>

typedef std::chrono::high_resolution_clock MainClock;

static const double BUFFER_DURATION = double(SAMPLES_PER_BUFFER) / SAMPLE_RATE;

static const MainClock::duration WORKER_INTERVAL = std::chrono::duration_cast<MainClock::duration>(
    std::chrono::duration<double>(BUFFER_DURATION / 4.0)
);

static const double COMPENSAION_DELAY = BUFFER_DURATION * BUFFER_POOL_SIZE * 1.2;

AudioStream::AudioStream() :
    m_bufferPoolData(new uint16_t[SAMPLES_PER_BUFFER * BUFFER_POOL_SIZE]),
    m_timeStreamStarted(MainClock::now().time_since_epoch().count()),
    m_workBuffer(new float[SAMPLES_PER_BUFFER]),
    m_bufferTime(-COMPENSAION_DELAY)
{
    for(int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        m_bufferPool[i].id = i;
        m_bufferPool[i].data = m_bufferPoolData + SAMPLES_PER_BUFFER * i;
        m_inputBufferQueue.push(&m_bufferPool[i]);
    }
}

void AudioStream::playEvent(const SoundEvent& soundEvent)
{
    m_nbSounds++;
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, getTime());
}

void AudioStream::playEventAt(const SoundEvent& soundEvent, double seconds)
{
    m_nbSounds++;
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, seconds);
}

void AudioStream::playEventIn(const SoundEvent& soundEvent, double seconds)
{
    m_nbSounds++;
    std::unique_lock<std::mutex> lock(m_soundsMutex);
    m_activeSounds.emplace_front(soundEvent, getTime() + seconds);
}

const int16_t* AudioStream::getNextBuffer()
{
    if(m_outputBufferQueue.empty())
    {
        i_fillNextBuffers();
    }

    const uint16_t* nextData = m_outputBufferQueue.front()->data;
    m_inputBufferQueue.push(m_outputBufferQueue.front());
    m_outputBufferQueue.pop();

    return (const int16_t*) nextData;
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
    delete[] m_workBuffer;
}

AudioStream::TimedSoundEvent::TimedSoundEvent(const SoundEvent& p_event, double p_time) :
    event(p_event),
    timeToPlay(p_time) {}

#include <cstring>

void AudioStream::i_fillNextBuffers()
{
    while(m_inputBufferQueue.size() > 0)
    {
        Buffer& currentBuffer = *m_inputBufferQueue.front();
        memset(m_workBuffer, 0, SAMPLES_PER_BUFFER * sizeof(float));

        {
            std::unique_lock<std::mutex> lock1(m_soundsMutex);
            for(TimedSoundEvent& sound : m_activeSounds)
            {
                if(!sound.hasStarted && m_bufferTime < sound.timeToPlay && sound.timeToPlay < m_bufferTime + BUFFER_DURATION)
                {
                    sound.hasStarted = true;
                    int sampleStart = (sound.timeToPlay - m_bufferTime) * SAMPLE_RATE;
                    sound.event.audioProducer->addOntoSamples(m_workBuffer + sampleStart, SAMPLES_PER_BUFFER - sampleStart, sound.event.volume);
                    continue;
                }
                else if(sound.hasStarted)
                {
                    sound.event.audioProducer->addOntoSamples(m_workBuffer, SAMPLES_PER_BUFFER, sound.event.volume);
                }
            }
            m_activeSounds.remove_if([&](TimedSoundEvent& e)
            {
                if(e.event.audioProducer->hasExpired() || !e.hasStarted && e.timeToPlay < m_bufferTime)
                {
                    m_nbSounds--;
                    delete e.event.audioProducer;
                    return true;
                }
                return false;
            });
            if(getTime() - m_bufferTime > COMPENSAION_DELAY * 3.0)
            {
                m_activeSounds.clear();
                m_bufferTime  = getTime() - COMPENSAION_DELAY;
            }
        }

        for(int i = 0; i < SAMPLES_PER_BUFFER; i++)
        {
            currentBuffer.data[i] = std::max(-1.0f, std::min(m_workBuffer[i], 1.0f)) * 32760;
        }
        m_outputBufferQueue.push(&currentBuffer);
        m_inputBufferQueue.pop();
        m_bufferTime += BUFFER_DURATION;
    }
}

void AudioStream::resartStream()
{
    m_timeStreamStarted = MainClock::now().time_since_epoch().count();
    m_bufferTime = -COMPENSAION_DELAY;

    while(m_outputBufferQueue.size() > 0)
    {
        m_inputBufferQueue.push(m_outputBufferQueue.front());
        m_outputBufferQueue.pop();
    }
}
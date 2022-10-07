#include "engmsc/EngineAudio.hpp"

#include "engmsc/EnginePhysics.hpp"

#include "AudioStream.hpp"
#include "Logger.hpp"

#include "glm/glm.hpp"

static EMLogger m_logger("Engine Audio");

static double m_pos = 0.0;

std::deque<float> EMEngineAudio::m_engineLog;

static double i_sampleFromLog();

class EngineAudioProducer : public EMAudioProducer
{
private:
    EngineAudioProducer()
    {

    }

    static EngineAudioProducer* m_producer;
public:
    static EngineAudioProducer* getInstance()
    {
        if(!m_producer)
        {
            m_producer = new EngineAudioProducer();
        }

        return m_producer;
    }

    virtual size_t placeSamples(float* buffer, size_t bufferLen) override
    {
        for(size_t i = 0; i < bufferLen; i++)
        {
            float sample = float(rand()) / RAND_MAX - 0.5f;
            sample *= (float) (i_sampleFromLog() - 100000) / 2e6f;
            buffer[i] += sample;
            m_pos += m_pitch;
        }

        return 0;
    }

    virtual double getDuration() const override
    {
        return 0.0; 
    }

    virtual bool hasExpired() const override
    {
        return false;
    }

    virtual ~EngineAudioProducer()
    {
        m_producer = new EngineAudioProducer();
    }
};

EngineAudioProducer* EngineAudioProducer::m_producer = NULL;

EMAudioProducer* EMEngineAudio::getAudioProducer()
{
    return EngineAudioProducer::getInstance();
}

static double value = 0.0;
static double pos = 0.0;
static double nextTick = 0.0;
static double tickDelta = 1.0 / 2000.0;
static double i_sampleFromLog()
{
    pos += 1.0 / EMSAMPLE_RATE;
    double result = (nextTick - pos) / tickDelta;
    result = 1 - result;
    result = 0 < result ? result : 0;

    while(nextTick < pos)
    {
        value = EMEngineAudio::m_engineLog.front();
        EMEngineAudio::m_engineLog.pop_front();
        nextTick += tickDelta;
    }

    return value;
}
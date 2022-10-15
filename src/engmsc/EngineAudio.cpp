#include "engmsc/EngineAudio.hpp"

#include "engmsc/EnginePhysics.hpp"

#include "AudioStream.hpp"
#include "Logger.hpp"

#include <glm/glm.hpp>
#include <Iir.h>

static EMLogger m_logger("Engine Audio");

static Iir::Butterworth::HighPass highpass;

std::deque<EMEngineAudio::CylinderStatus> EMEngineAudio::m_engineLog;

EMEngineAudio::CylinderStatus m_cylStatuses[16];
static void i_nextSample(int numCylinders);

class EngineAudioProducer : public EMAudioProducer
{
private:
    EngineAudioProducer()
    {
        highpass.setup(EMSAMPLE_RATE, 20);

        for(EMEngineAudio::CylinderStatus& cylStatus : m_cylStatuses)
        {
            cylStatus.exhaustPressure = 0.0f;
            cylStatus.exhaustVelocity = 0.0f;
        }
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
        int numCylinders = (int) EMEnginePhysics::getEngineAssembly().cylinders.size();
        assert(0 < numCylinders);

        for(size_t i = 0; i < bufferLen; i++)
        {
            i_nextSample(numCylinders);
            float value = 0.0f;
            for(int cyl = 0; cyl < numCylinders; cyl++)
            {
                auto cylStatus = m_cylStatuses[cyl];

                float r = float(rand()) / RAND_MAX - 0.5f;
                float sample = (cylStatus.exhaustPressure - 101325) * 5e-5f;
                sample += r * cylStatus.exhaustVelocity * 1e-1f;
                value += sample;
            }
            buffer[i] += highpass.filter(value);
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

static double pos = 0.0;
static double nextTick = 0.0;
static double tickDelta = 0.0;

static void i_nextSample(int numCylinders)
{
    pos += 1.0 / EMSAMPLE_RATE;
    
    while(nextTick < pos)
    {
        if(!EMEngineAudio::m_engineLog.empty())
        {
            for(int cyl = 0; cyl < numCylinders; cyl++)
            {
                m_cylStatuses[cyl] = EMEngineAudio::m_engineLog.front();
                EMEngineAudio::m_engineLog.pop_front();
                tickDelta = m_cylStatuses[cyl].timeDelta;
            }
        }
        nextTick += tickDelta;
    }
}
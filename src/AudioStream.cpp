#include "AudioStream.hpp"

#include "GLFWInclude.hpp"

#include <glm/glm.hpp>

static const double m_bufferDuration = double(EMSAMPLES_PER_BUFFER) / EMSAMPLE_RATE;

static const double m_compensationDelay = m_bufferDuration * EMBUFFER_POOL_SIZE * 1.2;

EMAudioStream::EMAudioStream() :
    m_buffersData(new data[EMSAMPLES_PER_BUFFER * EMBUFFER_POOL_SIZE]),
    m_timeStreamStarted(glfwGetTime()),
    m_bufferTime(-m_compensationDelay),
    m_nbEvents(0)
{
    for(int i = 0; i < EMBUFFER_POOL_SIZE; i++)
    {
        m_buffers[i].id = i;
        m_buffers[i].data = m_buffersData.get() + EMSAMPLES_PER_BUFFER * i;
        m_inputQueue.push(&m_buffers[i]);
    }
}

EMAudioStream::~EMAudioStream()
{
    for(EMAudioEvent& event : m_events)
    {
        delete event.m_audioProducer;
    }
}

void EMAudioStream::play(const EMAudioEvent& event)
{
    m_nbEvents++;
    std::unique_lock<std::mutex> lock(m_streamMutex);
    m_events.emplace_front(event);
    m_events.front().m_startTime = glfwGetTime();
}

void EMAudioStream::playIn(const EMAudioEvent& event, double seconds)
{
    m_nbEvents++;
    std::unique_lock<std::mutex> lock(m_streamMutex);
    m_events.emplace_front(event);
    m_events.front().m_startTime = glfwGetTime() + seconds;
}

void EMAudioStream::playAt(const EMAudioEvent& event, double seconds)
{
    m_nbEvents++;
    std::unique_lock<std::mutex> lock(m_streamMutex);
    m_events.emplace_front(event);
    m_events.front().m_startTime = seconds;
}

const EMAudioStream::data* EMAudioStream::getNextBuffer()
{
    if(m_outputQueue.empty())
    {
        fillNextBuffers();
    }

    const data* nextData = m_outputQueue.front()->data;
    m_inputQueue.push(m_outputQueue.front());
    m_outputQueue.pop();

    return nextData;
}

double EMAudioStream::getTime()
{
    return glfwGetTime() - m_timeStreamStarted;
}

int EMAudioStream::getNbEvents() const
{
    return m_nbEvents;
}

void EMAudioStream::restart()
{
    m_timeStreamStarted = glfwGetTime();
    m_bufferTime = -m_compensationDelay;

    while(!m_outputQueue.empty())
    {
        m_inputQueue.push(m_outputQueue.front());
        m_outputQueue.pop();
    }
}

void EMAudioStream::fillNextBuffers()
{
    while(!m_inputQueue.empty())
    {
        Buffer& currentBuffer = *m_inputQueue.front();
        m_workBuffer.fill(0.0f);

        {
            // Produce samples from audio events onto buffer
            std::unique_lock<std::mutex> lock(m_streamMutex);
            for(EMAudioEvent& event : m_events)
            {
                if(!event.m_hasStarted &&
                m_bufferTime < event.m_startTime && event.m_startTime < m_bufferTime + m_bufferDuration)
                {
                    event.m_hasStarted = true;
                    int sampleStart = (int) (event.m_startTime - m_bufferTime) * EMSAMPLE_RATE;

                    event.m_audioProducer->placeSamples(
                    m_workBuffer.data() + sampleStart, EMSAMPLES_PER_BUFFER - sampleStart);
                    continue;
                }
                else if(event.m_hasStarted)
                {
                    event.m_audioProducer->placeSamples(m_workBuffer.data(), EMSAMPLES_PER_BUFFER);
                }
            }

            // Remove expired events
            m_events.remove_if([&] (EMAudioEvent& event)
            {
                if(event.m_audioProducer->hasExpired() || !event.m_hasStarted && event.m_startTime < m_bufferTime)
                {
                    m_nbEvents--;
                    delete event.m_audioProducer;
                    return true;
                }
                return false;
            });

            // If falling behind, remove static audio events
            if(glfwGetTime() - m_bufferTime > m_compensationDelay * 3.0)
            {
                m_events.remove_if([&](EMAudioEvent& event)
                {
                    if(event.m_audioProducer->getDuration() > 0.0)
                    {
                        m_nbEvents--;
                        delete event.m_audioProducer;
                        return true;
                    }
                    return false;
                });

                m_bufferTime = glfwGetTime() - m_compensationDelay;
            }
        }

        // Convert float to s16
        for(int i = 0; i < EMSAMPLES_PER_BUFFER; i++)
        {
            float sample = m_workBuffer[i];
            sample = glm::max(-1.0f, std::min(sample, 1.0f));
            currentBuffer.data[i] = data(sample * 32767);
        }

        // Carry on with queue
        m_outputQueue.push(&currentBuffer);
        m_inputQueue.pop();
        m_bufferTime += m_bufferDuration;
    }
}
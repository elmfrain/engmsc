#include "AudioStream.hpp"

#include "GLFWInclude.hpp"

#include <glm/glm.hpp>

static const double m_bufferDuration = double(EMSAMPLES_PER_BUFFER) / EMSAMPLE_RATE;

static const double m_compensationDelay = m_bufferDuration * EMBUFFER_POOL_SIZE * 1.2;

EMAudioInsert::EMAudioInsert() :
    EMAudioInsert(0)
{

}

EMAudioInsert::EMAudioInsert(int id) :
    m_id(id),
    m_gain(1.0f)
{

}

EMAudioInsert::~EMAudioInsert()
{
    for(EMAudioFilter* filter : m_filters)
    {
        delete filter;
    }
    for(EMAudioEvent& event : m_events)
    {
        delete event.getProducer();
    }
}

float EMAudioInsert::getGain() const
{
    return m_gain;
}

void EMAudioInsert::setGain(float gain)
{
    m_gain = gain;
}

void EMAudioInsert::addFilter(EMAudioFilter* filter)
{
    m_filters.push_back(filter);
}

bool EMAudioInsert::removeFilter(EMAudioFilter* filter)
{
    for(EMAudioFilter* f : m_filters)
    {
        if(f == filter)
        {
            delete f;
            return true;
        }
    }
    return false;
}

size_t EMAudioInsert::numFilters() const
{
    return m_filters.size();
}

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

    m_inserts.emplace_back();
}

EMAudioStream::~EMAudioStream()
{
    for(EMAudioInsert& insert : m_inserts)
    for(EMAudioEvent& event : insert.m_events)
    {
        delete event.m_audioProducer;
    }
}

void EMAudioStream::play(const EMAudioEvent& event, int insertIndex)
{
    if(m_inserts.size() <= insertIndex || insertIndex < 0) insertIndex = 0;

    m_nbEvents++;
    std::unique_lock<std::mutex> lock(m_streamMutex);

    auto& eventList = m_inserts[insertIndex].m_events;
    eventList.emplace_front(event);
    eventList.front().m_startTime = glfwGetTime();
}

void EMAudioStream::playIn(const EMAudioEvent& event, double seconds, int insertIndex)
{
    if(m_inserts.size() <= insertIndex || insertIndex < 0) insertIndex = 0;

    m_nbEvents++;
    std::unique_lock<std::mutex> lock(m_streamMutex);

    auto& eventList = m_inserts[insertIndex].m_events;
    eventList.emplace_front(event);
    eventList.front().m_startTime = glfwGetTime() + seconds;
}

void EMAudioStream::playAt(const EMAudioEvent& event, double seconds, int insertIndex)
{
    if(m_inserts.size() <= insertIndex || insertIndex < 0) insertIndex = 0;

    m_nbEvents++;
    std::unique_lock<std::mutex> lock(m_streamMutex);

    auto& eventList = m_inserts[insertIndex].m_events;
    eventList.emplace_front(event);
    eventList.front().m_startTime = seconds;
}

EMAudioInsert& EMAudioStream::newInsert()
{
    return m_inserts.emplace_back((int) m_inserts.size());
}

EMAudioInsert& EMAudioStream::getInsert(int id)
{
    return m_inserts[id];
}

EMAudioInsert& EMAudioStream::getMainInsert()
{
    return getInsert(0);
}

void EMAudioStream::removeInsert(int id)
{
    assert(id != 0);

    m_inserts.erase(m_inserts.begin() + id);
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
            std::unique_lock<std::mutex> lock(m_streamMutex);

            // Produce samples from audio events from each insert,
            // apply filters, and add onto work buffer
            for(EMAudioInsert& insert : m_inserts)
            {
                m_insertWorkBuffer.fill(0.0f);
                m_filterWorkBuffer.fill(0.0f);

                for(EMAudioEvent& event : insert.m_events)
                {
                    if(!event.m_hasStarted &&
                    m_bufferTime < event.m_startTime && event.m_startTime < m_bufferTime + m_bufferDuration)
                    {
                        event.m_hasStarted = true;
                        int sampleStart = (int) (event.m_startTime - m_bufferTime) * EMSAMPLE_RATE;
    
                        event.m_audioProducer->placeSamples(
                        m_insertWorkBuffer.data() + sampleStart, EMSAMPLES_PER_BUFFER - sampleStart);
                        continue;
                    }
                    else if(event.m_hasStarted)
                    {
                        event.m_audioProducer->placeSamples(
                        m_insertWorkBuffer.data(), EMSAMPLES_PER_BUFFER);
                    }
                }

                float* inBuffer = m_insertWorkBuffer.data();
                float* outBuffer = m_filterWorkBuffer.data();
                for(EMAudioFilter* filter : insert.m_filters)
                {
                    filter->filter(outBuffer, inBuffer, EMSAMPLES_PER_BUFFER);

                    // Swap buffers
                    float* temp = inBuffer;
                    inBuffer = outBuffer;
                    outBuffer = temp;
                }

                for(int i = 0; i < EMSAMPLES_PER_BUFFER; i++)
                {
                    m_workBuffer[i] += inBuffer[i];
                }
            }

            // Remove expired events or let late events play
            for(EMAudioInsert& insert: m_inserts)
            insert.m_events.remove_if([&] (EMAudioEvent& event)
            {
                if(event.m_audioProducer->hasExpired())
                {
                    m_nbEvents--;
                    delete event.m_audioProducer;
                    return true;
                }
                else if(!event.m_hasStarted && event.m_startTime < m_bufferTime)
                {
                    event.m_hasStarted = true;
                }
                return false;
            });

            // If falling behind, remove non-static audio events
            // Note: Static events are known for their producer having a duration
            // of 0.0
            if(glfwGetTime() - m_bufferTime > m_compensationDelay * 2.0)
            {
                for(EMAudioInsert& insert: m_inserts)
                insert.m_events.remove_if([&](EMAudioEvent& event)
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
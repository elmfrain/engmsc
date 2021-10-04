#include <engmsc/WindProducer.hpp>
#include <engmsc/AudioStream.hpp>

size_t WindProducer::produceSamples(float* buffer, size_t bufferSize)
{
    m_lowPass.setup(SAMPLE_RATE, m_windVelocity * 4.25);

    for(size_t i = 0; i < bufferSize; i++)
    {
        double rnd = 2.0 * (double(rand()) / RAND_MAX) - 1.0f;
        
        buffer[i] = m_lowPass.filter(rnd) * std::min(m_windVelocity / 150.0, 0.9);
    }

    return bufferSize;
}

size_t WindProducer::addOntoSamples(float* buffer, size_t bufferSize, float gain)
{
    m_lowPass.setup(SAMPLE_RATE, m_windVelocity * 4.25);

    for(size_t i = 0; i < bufferSize; i++)
    {
        double rnd = 2.0 * (double(rand()) / RAND_MAX) - 1.0f;
        
        buffer[i] += m_lowPass.filter(rnd) * std::min(m_windVelocity / 150.0, 0.9);
    }

    return bufferSize;
}

double WindProducer::getDuration() const
{
    return 0.0;
}

bool WindProducer::hasExpired() const
{
    return m_expired;
}

void WindProducer::setWindVelocity(double velocity)
{
    m_windVelocity = velocity;
}

void WindProducer::expire()
{
    m_expired = true;
}
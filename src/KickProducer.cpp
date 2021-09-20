#include <engmsc/KickProducer.hpp>
#include <engmsc/AudioStream.hpp>
#include <math.h>

size_t KickProducer::produceSamples(float* buffer, size_t nbSamples)
{
    if(double(m_samplePos + nbSamples) / SAMPLE_RATE > getDuration())
    {
        nbSamples = getDuration() * SAMPLE_RATE - m_samplePos;
        m_hasExpired = true;
    }

    for(size_t i = 0; i < nbSamples; i++)
    {
        buffer[i] = genSample();
        m_samplePos++;
    }

    return nbSamples;
}

size_t KickProducer::addOntoSamples(float* buffer, size_t nbSamples)
{
    if(double(m_samplePos + nbSamples) / SAMPLE_RATE > getDuration())
    {
        nbSamples = getDuration() * SAMPLE_RATE - m_samplePos;
        m_hasExpired = true;
    }

    for(size_t i = 0; i < nbSamples; i++)
    {
        buffer[i] += genSample();
        m_samplePos++;
    }

    return nbSamples;
}

double KickProducer::getDuration() const
{
    return 1;
}

bool KickProducer::hasExpired() const
{
    return m_hasExpired;
}

#define F(x) (1000.0 / (10 * double(x) / SAMPLE_RATE + 1))
#define G(x) (1.0 / (10.0 * double(x) / SAMPLE_RATE + 1))

float KickProducer::genSample() const
{
    float noise = 2.0f * float(rand()) / RAND_MAX - 1.0f;
    float sample = sin((m_samplePos * 3.1415 * (80 + F(m_samplePos * 10))) / 44100) * 0.92f * G(m_samplePos * 6);
    sample += noise * 0.15f * G(m_samplePos * 20);

    return sample;
}
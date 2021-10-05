#include <engmsc/KickProducer.hpp>
#include <engmsc/AudioStream.hpp>
#include <math.h>

KickProducer::KickProducer(float factor, float factor2) :
    m_factor(80.0f + factor * 600.0f),
    m_factor2(20.0f + factor2 * 180.0f) {}

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

size_t KickProducer::addOntoSamples(float* buffer, size_t nbSamples, float gain)
{
    if(double(m_samplePos + nbSamples) / SAMPLE_RATE > getDuration())
    {
        nbSamples = getDuration() * SAMPLE_RATE - m_samplePos;
        m_hasExpired = true;
    }

    for(size_t i = 0; i < nbSamples; i++)
    {
        buffer[i] += genSample() * gain;
        m_samplePos++;
    }

    return nbSamples;
}

double KickProducer::getDuration() const
{
    return 0.25;
}

bool KickProducer::hasExpired() const
{
    return m_hasExpired;
}

const double SAMPLE_DURATION = 1.0 / SAMPLE_RATE;
const float PI = M_PI;

#define F(x) (m_factor / (10 * double(x) / SAMPLE_RATE + 1))
#define G(x) (1.0 / (10.0 * double(x) / SAMPLE_RATE + 1))

float KickProducer::genSample() const
{
    float noise = 2.0f * float(rand()) / RAND_MAX - 1.0f;
    float sample = sin((m_samplePos * 3.1415 * (50.0 + F(m_samplePos * 50))) / 44100) * 0.276f * G(m_samplePos * 6);
    sample += noise * 0.045f * G(m_samplePos * m_factor2 * 2.0f);
    sample *= 1.0f - (m_samplePos * SAMPLE_DURATION) / getDuration();

    return sample;
}
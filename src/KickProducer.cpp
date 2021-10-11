#include <engmsc/KickProducer.hpp>
#include <engmsc/AudioStream.hpp>
#include <math.h>

KickProducer::KickProducer(float factor, float factor2, float factor3) :
    m_factor(80.0f + factor * 600.0f),
    m_factor2(20.0f + factor2 * 180.0f),
    m_duration(std::max(0.02f, std::min(factor3, 1.0f))) {}

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
    return m_duration;
}

bool KickProducer::hasExpired() const
{
    return m_hasExpired;
}

const double SAMPLE_DURATION = 1.0 / SAMPLE_RATE;
const float PI = 3.14159265f;

#define sg 10.0
#define F(x) (m_factor / (10 * double(x) / SAMPLE_RATE + 1))
#define G(x) (1.0 / (10.0 * double(x) / SAMPLE_RATE + 1))
#define S(x) ( (sg * x) / (1.0 + abs(sg * x)) )

float KickProducer::genSample() const
{
    float noise = 2.0f * float(rand()) / RAND_MAX - 1.0f;
    float sine = sin((m_samplePos * 3.1415 * (50.0 + F(m_samplePos * 50))) / 44100);
    float sample = sine * 0.276f * G(m_samplePos * 6);
    sample += noise * 0.045f * G(m_samplePos * m_factor2 * 2.0f);
    sample *= (0.2f + m_factor / 4000.0f);
    sample *= 1.0f - (m_samplePos * SAMPLE_DURATION) / getDuration();

    return sample;
}
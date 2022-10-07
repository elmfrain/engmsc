#include "AudioEvent.hpp"

EMAudioEvent::EMAudioEvent(EMAudioProducer* producer, float gain, float pitch) :
    m_audioProducer(producer),
    m_gain(gain),
    m_pitch(pitch),
    m_samplePos(0.0)
{

}

float EMAudioEvent::getGain() const
{
    return m_gain;
}

float EMAudioEvent::getPitch() const
{
    return m_pitch;
}

EMAudioProducer* EMAudioEvent::getProducer()
{
    return m_audioProducer;
}

void EMAudioEvent::setGain(float gain)
{
    m_gain = gain;
}

void EMAudioEvent::setPitch(float pitch)
{
    m_pitch = pitch;
}
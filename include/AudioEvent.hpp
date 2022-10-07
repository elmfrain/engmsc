#ifndef EMAUDIO_EVENT_HPP
#define EMAUDIO_EVENT_HPP

#include "AudioProducer.hpp"

class EMAudioEvent
{
public:
    EMAudioEvent(EMAudioProducer* producer, float gain = 1.0f, float pitch = 1.0f);

    float getGain() const;
    float getPitch() const;
    EMAudioProducer* getProducer();

    void setGain(float gain);
    void setPitch(float pitch);
private:
    friend class EMAudioStream;

    float m_gain;
    float m_pitch;
    EMAudioProducer* m_audioProducer;
    double m_startTime;
    bool m_hasStarted;
};

#endif // EMAUDIO_EVENT_HPP
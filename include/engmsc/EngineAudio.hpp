#ifndef EMENGINE_AUDIO_HPP
#define EMENGINE_AUDIO_HPP

#include "AudioProducer.hpp"

#include <deque>

namespace EMEngineAudio
{
    struct CylinderStatus
    {
        double timeDelta = 0.0;
        float exhaustPressure = 0.0f;
        float exhaustVelocity = 0.0f;
    };

    // Only to be filled by EMEnginePhysics
    extern std::deque<CylinderStatus> m_engineLog;

    EMAudioProducer* getAudioProducer();

    void update();
}


#endif // EMENGINE_AUDIO_HPP
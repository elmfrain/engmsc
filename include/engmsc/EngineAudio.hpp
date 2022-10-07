#ifndef EMENGINE_AUDIO_HPP
#define EMENGINE_AUDIO_HPP

#include "AudioProducer.hpp"

#include <deque>

namespace EMEngineAudio
{
    // Only to be filled by EMEnginePhysics
    extern std::deque<float> m_engineLog;

    EMAudioProducer* getAudioProducer();

    void update();
}


#endif // EMENGINE_AUDIO_HPP
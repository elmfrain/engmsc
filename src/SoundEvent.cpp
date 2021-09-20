#include <engmsc/SoundEvent.hpp>

SoundEvent::SoundEvent(IAudioProducer* producer, float volume, float pitch) :
    audioProducer(producer),
    volume(volume),
    pitch(pitch) {}
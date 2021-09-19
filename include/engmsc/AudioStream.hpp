#pragma once

#ifndef AUDIO_STREAM_HPP
#define AUDIO_STREAM_HPP

#include <inttypes.h>
#include <engmsc/SoundEvent.hpp>

#include <forward_list>

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

#define SAMPLE_RATE 44100
#define SAMPLES_PER_BUFFER 4098
#define BUFFER_POOL_SIZE 3

class AudioStream
{
public:
    AudioStream();

    void playEvent(const SoundEvent& event);
    void playEventAt(const SoundEvent& event, double seconds);
    void playEventIn(const SoundEvent& event, double seconds);
    const int16_t* getNextBuffer();
    bool hasNextReady();
    double getTime();

    AudioStream(const AudioStream& copy) = delete;
    AudioStream operator=(const AudioStream& copy) = delete;

    ~AudioStream();
private:
    struct Buffer
    {
        int id = 0;
        uint16_t* data = nullptr;
    };
    struct TimedSoundEvent
    {
        TimedSoundEvent(const SoundEvent& event, double time);
        SoundEvent event;
        double timeToPlay = 0.0;
    };

    std::mutex m_soundsMutex;
    std::forward_list<TimedSoundEvent> m_activeSounds;

    std::mutex m_bufferQueueMutex;
    std::queue<Buffer*> m_outputBufferQueue;
    std::queue<Buffer*> m_inputBufferQueue;

    uint16_t* const m_bufferPoolData;
    Buffer m_bufferPool[BUFFER_POOL_SIZE];

    const size_t m_timeStreamStarted;

    //Buffering thread variables
    std::mutex m_threadMutex;
    std::condition_variable m_bufferingThreadCV;
    bool m_exitThread = false;
    std::thread* m_bufferingThread;
    void i_bufferingThread();
};

#endif
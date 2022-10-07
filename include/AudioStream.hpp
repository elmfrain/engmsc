#ifndef EMAUDIO_STREAM_HPP
#define EMAUDIO_STREAM_HPP

#include "AudioEvent.hpp"

#include <inttypes.h>
#include <forward_list>
#include <queue>
#include <array>
#include <memory>
#include <mutex>

// Buffering Settings
#ifndef EMSAMPLE_RATE
#define EMSAMPLE_RATE 44100
#endif

#ifndef EMSAMPLES_PER_BUFFER
#define EMSAMPLES_PER_BUFFER 1024
#endif

#ifndef EMBUFFER_POOL_SIZE
#define EMBUFFER_POOL_SIZE 3
#endif

class EMAudioStream
{
public:
    typedef int16_t data;

    EMAudioStream();
    EMAudioStream(const EMAudioStream& copy) = delete;
    ~EMAudioStream();

    void play(const EMAudioEvent& event);
    void playIn(const EMAudioEvent& event, double seconds);
    void playAt(const EMAudioEvent& event, double seconds);

    const data* getNextBuffer();
    double getTime();
    int getNbEvents() const;
    void restart();
private:
    struct Buffer
    {
        int id;
        data* data;
    };

    int m_nbEvents;
    
    std::forward_list<EMAudioEvent> m_events;
    std::mutex m_streamMutex;

    std::queue<Buffer*> m_outputQueue;
    std::queue<Buffer*> m_inputQueue;

    Buffer m_buffers[EMBUFFER_POOL_SIZE];

    double m_timeStreamStarted;

    std::unique_ptr<data> m_buffersData;
    std::array<float, EMSAMPLES_PER_BUFFER> m_workBuffer;
    double m_bufferTime;
    void fillNextBuffers();
};

#endif // EMAUDIO_STREAM_HPP
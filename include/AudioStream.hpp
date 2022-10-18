#ifndef EMAUDIO_STREAM_HPP
#define EMAUDIO_STREAM_HPP

#include "AudioEvent.hpp"
#include "AudioFilter.hpp"

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
#define EMSAMPLES_PER_BUFFER 512
#endif

#ifndef EMBUFFER_POOL_SIZE
#define EMBUFFER_POOL_SIZE 4
#endif

class EMAudioInsert
{
public:
    EMAudioInsert();
    EMAudioInsert(int id);

    float getGain() const;
    void setGain(float gain);

    void addFilter(EMAudioFilter* filter);
    bool removeFilter(EMAudioFilter* filter);
    size_t numFilters() const;

    ~EMAudioInsert();
private:
    int m_id;
    float m_gain;

    std::forward_list<EMAudioEvent> m_events;
    std::vector<EMAudioFilter*> m_filters;

    friend class EMAudioStream;
};

class EMAudioStream
{
public:
    typedef int16_t data;

    EMAudioStream();
    EMAudioStream(const EMAudioStream& copy) = delete;
    ~EMAudioStream();

    void play(const EMAudioEvent& event, int insertIndex = 0);
    void playIn(const EMAudioEvent& event, double seconds, int insertIndex = 0);
    void playAt(const EMAudioEvent& event, double seconds, int insertIndex = 0);

    EMAudioInsert& newInsert();
    EMAudioInsert& getInsert(int id);
    EMAudioInsert& getMainInsert();
    void removeInsert(int id);

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
    
    std::vector<EMAudioInsert> m_inserts;
    std::mutex m_streamMutex;

    std::queue<Buffer*> m_outputQueue;
    std::queue<Buffer*> m_inputQueue;

    Buffer m_buffers[EMBUFFER_POOL_SIZE];

    double m_timeStreamStarted;

    std::unique_ptr<data> m_buffersData;
    std::array<float, EMSAMPLES_PER_BUFFER> m_workBuffer;
    std::array<float, EMSAMPLES_PER_BUFFER> m_insertWorkBuffer;
    std::array<float, EMSAMPLES_PER_BUFFER> m_filterWorkBuffer;
    double m_bufferTime;
    void fillNextBuffers();
};

#endif // EMAUDIO_STREAM_HPP
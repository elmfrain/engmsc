#include "engmscapi.h"
#include <mutex>
#include <condition_variable>
#include <map>

using namespace engmsc;

AudioStream::AudioStream(uint8_t channels, bool allowStereo, size_t sampleRate, size_t samplesPerBuffer, size_t bufferPoolSize)
{
	AVSampleFormat defaultFormat = AV_SAMPLE_FMT_FLTP;
	sample_rate = sampleRate;
	buffer_size = samplesPerBuffer;
	buffer_pool_size = bufferPoolSize > 4 ? bufferPoolSize : 4;
	allow_stereo = allowStereo;
	nb_sources = allowStereo && channels == 2 ? 1 : channels;
	nb_channels = channels;

	chnlLayout = 0xFFFFFFFFFFFFFFFF;
	int swrDstLineSize;
	if (channels < 64)
	{
		chnlLayout = chnlLayout >> (64 - channels);
	}
	else
	{
		std::cout << "[AudioStream] : Cannot have more than 64 channels!";
	}
	av_samples_alloc_array_and_samples(&swrDstData, &swrDstLineSize, channels, samplesPerBuffer, AV_SAMPLE_FMT_S16P, 0);
	swrContext = swr_alloc();
	av_opt_set_int(swrContext, "in_channel_layout", chnlLayout, 0);
	av_opt_set_int(swrContext, "in_sample_rate", sampleRate, 0);
	av_opt_set_sample_fmt(swrContext, "in_sample_fmt", defaultFormat, 0);

	av_opt_set_int(swrContext, "out_channel_layout", chnlLayout, 0);
	av_opt_set_int(swrContext, "out_sample_rate", sampleRate, 0);
	av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16P, 0);
	swr_init(swrContext);

	bufferPool = new ALUtil::AudioBuffer[buffer_pool_size];
	for (int buffer = 0; buffer < buffer_pool_size; buffer++)
	{
		bufferPool[buffer] = ALUtil::AudioBuffer(samplesPerBuffer, sampleRate, defaultFormat, channels);
	}

	alSources = new unsigned int[nb_sources];
	alGenSources(nb_sources, alSources);
	stream = std::thread([this]() { streamThread(); });
	converter = std::thread([this]() {converterThread(); });
}
AudioStream::~AudioStream()
{
	if (!joined)
	{
		running = false;
		stream.join();
		condVar.notify_one();
		converter.join();
	}
	audBufCache.clear();
	delete[] bufferPool;
	swr_free(&swrContext);
	ALUtil::deleteAVBuffer(swrDstData);
	alDeleteSources(nb_sources, alSources);
	delete[] alSources;
}
void AudioStream::close()
{
	running = false;
	stream.join();
	condVar.notify_one();
	converter.join();
	joined = true;
}
void AudioStream::playSound(SoundEvent e)
{
	event newEvent;
	newEvent.startTime = std::chrono::high_resolution_clock::now();
	newEvent.aquisitionTime = newEvent.startTime;
	newEvent.soundEvent = e;
	std::lock_guard<std::mutex> lock(conversionMutex);
	conversionQueue.push(newEvent);
	condVar.notify_one();
}
void AudioStream::playSoundAt(SoundEvent e, std::chrono::high_resolution_clock::time_point timePoint)
{
	event newEvent;
	newEvent.startTime = timePoint;
	newEvent.aquisitionTime = std::chrono::high_resolution_clock::now();
	newEvent.soundEvent = e;
	std::lock_guard<std::mutex> lock(conversionMutex);
	conversionQueue.push(newEvent);
	condVar.notify_one();
}
void AudioStream::playSoundIn(SoundEvent e, double seconds)
{
	using namespace std::chrono_literals;
	event newEvent;
	newEvent.aquisitionTime = std::chrono::high_resolution_clock::now();
	newEvent.startTime = newEvent.aquisitionTime + std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(seconds * 1.0s);
	newEvent.soundEvent = e;
	std::lock_guard<std::mutex> lock(conversionMutex);
	conversionQueue.push(newEvent);
	condVar.notify_one();
}
void AudioStream::play()
{
	int state;
	alGetSourcei(*alSources, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING)
	{
		alSourcePlayv(nb_sources, alSources);
		paused = false;
	}
}
void AudioStream::pause()
{
	int state;
	alGetSourcei(*alSources, AL_SOURCE_STATE, &state);
	if (state != AL_PAUSED)
	{
		alSourcePausev(nb_sources, alSources);
		paused = true;
	}
}
void AudioStream::stop()
{
	if (!paused)
	{
		paused = true;
		{
			std::lock_guard<std::mutex> lock(listMutex);
			pendingEvents.clear();
		}
		alSourceStopv(nb_sources, alSources);
	}
}
void AudioStream::converterThread()
{
	using namespace std::chrono_literals;
	using namespace std::chrono;
	while (running)
	{
		std::unique_lock<std::mutex> locker(mu);
		condVar.wait(locker, [this]() {return !running || !conversionQueue.empty(); });
		while (!conversionQueue.empty())
		{
			event e = conversionQueue.front();

			//If sound event uses an audiobuffer.
			if(e.soundEvent.type == BUFFERED)
			{
				bufCache* theCache = nullptr;
				e.soundDuration = duration_cast<high_resolution_clock::duration>((double(e.soundEvent.buffer->samples) / e.soundEvent.buffer->sampleRate) * 1.0s);
				//Determines if the sound will ever play
				if (e.startTime + e.soundDuration < std::chrono::high_resolution_clock::now())
				{
					std::lock_guard<std::mutex> lock(conversionMutex);
					conversionQueue.pop();
					continue;
				}
				if (e.soundEvent.pitch == 0.0)
				{
					std::lock_guard<std::mutex> lock(conversionMutex);
					conversionQueue.pop();
					continue;
				}
				//Looks if the audiobuffer is cached
				for (bufCache& cache : audBufCache)
				{
					if (e.soundEvent.buffer == cache.original) { theCache = &cache; break; }
				}
				//Convert the audiobuffer if it isnt cached, otherwise use the cached one
				high_resolution_clock::duration conversionTime = 0s;
				if (theCache == nullptr)
				{
					auto start = high_resolution_clock::now();
					audBufCache.emplace_front(e.soundEvent.buffer, nb_channels, sample_rate, AV_SAMPLE_FMT_FLTP);
					if (audBufCache.size() > maxCacheSize) audBufCache.pop_back();
					e.buffer = &audBufCache.front().buffer;
					auto end = high_resolution_clock::now();
					conversionTime = end - start;

					if(e.aquisitionTime == e.startTime) e.startTime = std::chrono::high_resolution_clock::now();
				}
				else
				{
					e.buffer = &theCache->buffer;
				}
				double samplesToSkip = (std::max<high_resolution_clock::duration>(0s, e.aquisitionTime - e.startTime + conversionTime).count() / 1e9) * sample_rate;
				if (e.soundEvent.reversed) e.samplePos = e.buffer->samples - 1 - samplesToSkip;
				else e.samplePos = samplesToSkip;

				pendingEvents.push_back(e);
			}
			if (e.soundEvent.type == SYNTHEZIZED)
			{
				if (e.soundEvent.pitch == 0.0)
				{
					std::lock_guard<std::mutex> lock(conversionMutex);
					conversionQueue.pop();
					continue;
				}
				pendingEvents.push_back(e);
			}
			
			{
				std::lock_guard<std::mutex> lock(conversionMutex);
				conversionQueue.pop();
			}
		}
	}
}
void AudioStream::streamThread()
{
	using namespace std::literals::chrono_literals;
	using namespace std::chrono;
	std::mutex streamMutex;
	std::condition_variable timer;

	struct runningbuffer
	{
		ALUtil::AudioBuffer* buffer = nullptr;
		unsigned int* alBuffers = nullptr;
		void clear()
		{
			if (buffer != nullptr)
			{
				for (int channel = 0; channel < buffer->channels; channel++)
				{
					memset(buffer->data[channel], 0, buffer->bytesPerChannel);
				}
			}
		}
	};

	//Buffer init
	runningbuffer* runningBuffers = new runningbuffer[buffer_pool_size];
	std::queue<runningbuffer*> pendingQueue;
	std::queue<runningbuffer*> playingQueue;
	std::queue<runningbuffer*> processingBuffers;

	int pendingSize = buffer_pool_size, playingSize = 0, processingSize = 0;
	for (int buffer = 0; buffer < buffer_pool_size; buffer++)
	{
		runningBuffers[buffer].buffer = &bufferPool[buffer];
		runningBuffers[buffer].alBuffers = new unsigned int[nb_sources];
		pendingQueue.push(&runningBuffers[buffer]);
	}

	auto elapse = high_resolution_clock::now();
	const high_resolution_clock::duration updateInterval = duration_cast<high_resolution_clock::duration>((double(buffer_size) / sample_rate) * 1.0s);
	const high_resolution_clock::duration audioBufferDuration = duration_cast<high_resolution_clock::duration>((double(buffer_size) / sample_rate) * 1.0s);
	const high_resolution_clock::duration sampleDuration = audioBufferDuration / buffer_size;
	const high_resolution_clock::duration audioLag = audioBufferDuration * buffer_pool_size + audioBufferDuration * 2;
	auto audioElapse = elapse - audioLag;

	//Loop
	while (running)
	{
		auto start = high_resolution_clock::now();
		//WritingStage
		while (!pendingQueue.empty())
		{
			runningbuffer* b = pendingQueue.front();

			b->clear();
			
			high_resolution_clock::time_point sampleTimePoint = audioElapse;
			for (int sample = 0; sample < b->buffer->samples; sample++)
			{
				for (event& i : pendingEvents)
				{
					if (!i.inUse && i.startTime < sampleTimePoint) i.inUse = true;
					if (!i.playedThrough() && i.inUse)
					{
						for (int channel = 0; channel < nb_channels; channel++)
						{
							float s = i.interpolate(channel, i.samplePos) * i.soundEvent.volume;
							((float*)b->buffer->data[channel])[sample] += s;
						}
						if (!i.soundEvent.reversed) i.samplePos += i.soundEvent.pitch * masterPitch;
						else i.samplePos -= i.soundEvent.pitch * masterPitch;
					}
				}
				sampleTimePoint += sampleDuration;
			}
			

			playingQueue.push(b);
			pendingQueue.pop();
			audioElapse += audioBufferDuration;
		}
		//Playing Stage
		while (!playingQueue.empty())
		{
			runningbuffer* b = playingQueue.front();
			
			createALBuffers(*b->buffer, b->alBuffers, nb_sources);
			queueBuffers(b->alBuffers);

			playingQueue.pop();
			processingBuffers.push(b);
		}
		//Processed Stage
		int nbOfBuffersUnqueued = deleteUnqueuedBuffers();
		for (int buffer = 0; buffer < nbOfBuffersUnqueued; buffer++)
		{
			pendingQueue.push(processingBuffers.front());
			processingBuffers.pop();
		}
		//Make sure sources are playing if not paused of stopped
		if (!paused)
		{
			for (int source = 0; source < nb_sources; source++)
			{
				int state;
				alGetSourcei(alSources[source], AL_SOURCE_STATE, &state);
				if (state != AL_PLAYING)
				{
					alE(alSourcePlay(alSources[source]));
					underuns++;
				}
			}
		}
		//Clear events that had been played through
		{
			std::lock_guard<std::mutex> lock(listMutex);
			pendingEvents.remove_if([](event& e) { return e.playedThrough(); });
		}
		auto end = high_resolution_clock::now();

		if (audioElapse < end - audioLag * 2)
		{
			audioElapse = end - audioLag;
			std::lock_guard<std::mutex> lock(listMutex);
			//pendingEvents.remove_if([=](event& e) { return audioElapse > e.startTime + e.soundDuration; });
		}
		if (end - start > updateInterval)
		{
			elapse = high_resolution_clock::now();
			//audioElapse = elapse - audioBufferDuration * buffer_pool_size - audioBufferDuration;
		}

		elapse += updateInterval;
		std::unique_lock<std::mutex> locker(streamMutex);
		timer.wait_until(locker, elapse, [this]() { return !running; });
	}

	//Buffer terminate
	for (int buffer = 0; buffer < buffer_pool_size; buffer++)
	{
		if (runningBuffers[buffer].alBuffers != nullptr) delete[] runningBuffers[buffer].alBuffers;
	}
	delete[] runningBuffers;
}
void AudioStream::queueBuffers(unsigned int* alBuffers)
{
	for (int source = 0; source < nb_sources; source++)
	{
		alE(alSourceQueueBuffers(alSources[source], 1, &alBuffers[source]);)
	}
}
int AudioStream::deleteUnqueuedBuffers()
{
	int totalNbUnqueuedBuffers = 0;
	for (int source = 0; source < nb_sources; source++)
	{
		unsigned int buffers[64];
		int buffersProcessed;
		alGetSourcei(alSources[source], AL_BUFFERS_PROCESSED, &buffersProcessed);
		alE(alSourceUnqueueBuffers(alSources[source], buffersProcessed, buffers);)
		alE(alDeleteBuffers(buffersProcessed, buffers));
		totalNbUnqueuedBuffers += buffersProcessed;
	}
	return totalNbUnqueuedBuffers / nb_sources;
}
void AudioStream::createALBuffers(const ALUtil::AudioBuffer& buffer, unsigned int* val, size_t n)
{
	swr_convert(swrContext, swrDstData, buffer.samples, (const uint8_t**)buffer.data, buffer.samples);

	if (allow_stereo && buffer.channels == 2 && n >= 1)
	{
		alE(alGenBuffers(1, val);)
			short* stereoBuffer = new short[buffer.samples * 2];
		int bufferPos = 0;
		for (int sample = 0; sample < buffer.samples; sample++)
		{
			stereoBuffer[bufferPos++] = ((short*)swrDstData[0])[sample];
			stereoBuffer[bufferPos++] = ((short*)swrDstData[1])[sample];
		}
		alE(alBufferData(val[0], AL_FORMAT_STEREO16, stereoBuffer, buffer.samples * sizeof(short) * 2, buffer.sampleRate);)
			delete[] stereoBuffer;
	}
	else
	{
		int buffersToProduce = n >= buffer.channels ? buffer.channels : n;
		alE(alGenBuffers(buffersToProduce, val);)
			for (int buf = 0; buf < buffersToProduce; buf++)
			{
				alE(alBufferData(val[buf], AL_FORMAT_MONO16, swrDstData[buf], buffer.samples * sizeof(short), buffer.sampleRate));
			}
	}
}
#pragma once

#include "elmspi/audio/ALUtil.h"
#include <al.h>
#include <chrono>
#include <mutex>
#include <thread>
#include <queue>
#include <list>
#include <functional>

namespace engmsc
{
	struct SoundEvent;
	typedef std::function<float(int, double, SoundEvent&)> SampleGenerator;
	typedef std::function<bool(double)> ToTerminate;
	enum EventType : char { UNKNOWN, BUFFERED, SYNTHEZIZED };
	struct Synthesizer
	{
		SampleGenerator genertor;
		ToTerminate terminator;
		Synthesizer() {}
		Synthesizer(SampleGenerator generator, ToTerminate terminator)
		{
			this->genertor = generator;
			this->terminator = terminator;
		}
	};
	struct SoundEvent
	{
		float pitch = 1.0f;
		float volume = 1.0f;
		bool reversed = false;
		ALUtil::AudioBuffer* buffer = nullptr;
		Synthesizer synth;
		EventType type = UNKNOWN;
		SoundEvent() {}
		SoundEvent(ALUtil::AudioBuffer& buf, float volume = 1.0f, float ptich = 1.0f)
		{
			buffer = &buf;
			type = BUFFERED;
			this->volume = volume;
			this->pitch = ptich;
		}
		SoundEvent(Synthesizer synth, float volume = 1.0f, float ptich = 1.0f)
		{
			this->synth = synth;
			type = SYNTHEZIZED;
			this->volume = volume;
			this->pitch = ptich;
		}
	};

	class AudioStream
	{
		private:
			unsigned int buffer_pool_size;
			unsigned int buffer_size;
			unsigned int sample_rate;
			unsigned int nb_sources;
			unsigned int nb_channels;
			bool allow_stereo;
			ALUtil::AudioBuffer* bufferPool;

			bool paused = false;

			SwrContext* swrContext;
			uint8_t** swrDstData;
			uint64_t chnlLayout;

			unsigned int* alSources;

			std::mutex mu;
			std::condition_variable condVar;
			bool running = true;
			bool joined = false;
			std::thread stream;
			std::thread converter;

			struct event
			{
				SoundEvent soundEvent;
				double samplePos = 0.0;
				ALUtil::AudioBuffer* buffer;
				bool inUse = false;
				std::chrono::high_resolution_clock::time_point startTime;
				std::chrono::high_resolution_clock::time_point aquisitionTime;
				std::chrono::high_resolution_clock::duration soundDuration;
				event() {}
				event(ALUtil::AudioBuffer* b) { buffer = b; }
				bool playedThrough() 
				{
					if (soundEvent.type == BUFFERED)
					{
						return samplePos < -256 || buffer->samples + 256 < samplePos;
					}
					if (soundEvent.type == SYNTHEZIZED)
					{
						return samplePos < -256 || soundEvent.synth.terminator(samplePos);
					}
					return true;
				}
				float interpolate(int channel, double wavePos)
				{
					if (soundEvent.type == BUFFERED)
					{
						int sPos1 = std::max<double>(0, std::min<double>(wavePos, buffer->samples - 1));
						int sPos2 = sPos1 + 1 >= buffer->samples ? sPos1 : sPos1 + 1;
						int sPos3 = sPos1 + 2 >= buffer->samples ? sPos1 : sPos1 + 2;
						int sPos4 = sPos1 + 3 >= buffer->samples ? sPos1 : sPos1 + 3;

						float s1 = ((float*)buffer->data[channel])[sPos1];
						float s2 = ((float*)buffer->data[channel])[sPos2];
						float s3 = ((float*)buffer->data[channel])[sPos3];
						float s4 = ((float*)buffer->data[channel])[sPos4];

						float a1, a2, a3, a4, frac = wavePos - int(wavePos), frac2 = frac * frac;

						a1 = s4 - s3 - s1 + s2;
						a2 = s1 - s2 - a1;
						a3 = s3 - s1;
						a4 = s2;

						return (a1 * frac * frac2 + a2 * frac2 + a3 * frac + a4); //Cubic
					}
					if (soundEvent.type == SYNTHEZIZED)
					{
						return soundEvent.synth.genertor(channel, wavePos, soundEvent);
					}
					return 0.0f;
				}
			};
			struct bufCache
			{
				ALUtil::AudioBuffer* original;
				ALUtil::AudioBuffer buffer;
				bufCache(ALUtil::AudioBuffer* old, size_t nbChannels, size_t sampleRate, AVSampleFormat fmt)
				{
					original = old;
					buffer = ALUtil::convertAudio(*old, nbChannels, sampleRate, fmt);
				}
			};

			const int maxCacheSize = 256;
			std::mutex listMutex;
			std::list<bufCache> audBufCache;
			std::list<event> pendingEvents;
			std::queue<event> conversionQueue;
			std::mutex conversionMutex;

			void streamThread();
			void converterThread();
			void queueBuffers(unsigned int* alBuffers);
			void createALBuffers(const ALUtil::AudioBuffer&, unsigned int* val, size_t n);
			/*Returns the number of buffers processed.*/
			int deleteUnqueuedBuffers();
		public:
			int underuns = -1;
			float masterPitch = 1.0f;
			AudioStream(uint8_t channels, bool allowStereo = false, size_t sampleRate = 44100, size_t samplesPerBuffer = 2048, size_t bufferPoolSize = 16);
			~AudioStream();

			void close();
			void playSound(SoundEvent event);
			void playSoundAt(SoundEvent event, std::chrono::high_resolution_clock::time_point timePoint);
			void playSoundIn(SoundEvent event, double timeInSeconds);
			void play();
			void pause();
			void stop();
			size_t bufferPoolSize() { return buffer_pool_size; }
			size_t samplesPerBuffer() { return buffer_size; }
			int sampleRate() { return sample_rate; }
	};

	struct EngineInfo
	{
		unsigned int cylinders = 1;
		/* In Liters */
		float displacement = 1.0f;
		/* In kg*m^2*/
		float rotationalInertia = 2.0f;
		float volumetricEfficiency = 1.0f;
		/* In n * m */
		float maxTorque = 400.0f;
	};

	static const char* thudSound = "rsc/sound/thud.mp3";
	class Engine
	{
	private:
		std::chrono::high_resolution_clock::time_point prevTime;
		std::chrono::high_resolution_clock::time_point time;
		double angularVelocity = 17.0f;
		double angle = 0.0f;
		double externalTorque_i = 0.0;
		float throttle_i = 0.0;

	public:
		EngineInfo engineInfo;
		Engine() 
		{
			prevTime = std::chrono::high_resolution_clock::now();
		}
		Engine(EngineInfo info)
		{
			engineInfo = info;
			prevTime = std::chrono::high_resolution_clock::now();
		}
		float getRPM()
		{
			return abs(angularVelocity * 60.0);
		}
		void setExternalTorque(double torque)
		{
			externalTorque_i = torque;
		}
		double getCurrentAngle()
		{
			return angle;
		}
		void throttle(float amount)
		{
			throttle_i = amount;
		}
		void tick()
		{
			prevTime = time;
			time = std::chrono::high_resolution_clock::now();
			double deltaTime = std::chrono::duration<double>(time - prevTime).count();

			float torque = throttle_i * engineInfo.maxTorque + externalTorque_i;
			float rotaionalAcclertaion = torque / engineInfo.rotationalInertia;
			
			angularVelocity += deltaTime * rotaionalAcclertaion;
			angle += angularVelocity;
		}
	};
}

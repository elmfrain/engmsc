#include <gl/glew.h>
#include <al.h>
#include "elmspi/window/Window.h"
#include "elmspi/gui/GUISlider.h"
#include "elmspi/media/AudioClip.h"
#include "gui/EngMuscGUI.h"
#include "api/engmscapi.h"
#include <thread>
#include <Windows.h>

using namespace engmsc;

BaseGUI* theGUI;
TurboDynoGUI* turboGUI;
float pitch = 1.0f;

void render(Window* window)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, window->getWidth(), window->getHeight());
	if (theGUI)
	{
		theGUI->update(*window);
		theGUI->render();
	}
	if (turboGUI)
	{
		turboGUI->update(*window);
		turboGUI->render();
	}
}

float randFloat()
{
	return float(rand()) / RAND_MAX;
}

void dyno()
{
	using namespace espi;
	using namespace std::chrono;
	using namespace std::chrono_literals;
	WindowUtil::audioThreadHint(true);
	Window window(1280, 720, "Engine Music");
	window.makeThisContextCurrent();
	window.setRenderCallBack(render);

	unsigned int source;
	unsigned int buffer;

	alGenSources(1, &source);

	//AudioClip audio("D:/Program Files/FLStudio/Data/Patches/Packs/Drums/Kicks/22in Kick.wav");
	AudioClip audio("rsc/sound/thud.mp3");
	audio.decodeAudio();
	ALUtil::createALBuffers(audio.getAudioBuffer(), &buffer, 1, true);
	alE(alSourcei(source, AL_BUFFER, buffer));

	engmsc::AudioStream audStream(1, true, 44100, 128, 16);

	EngineDynoGUI gui = EngineDynoGUI(window);
	theGUI = &gui;

	auto elapse = high_resolution_clock::now();
	auto interval = duration_cast<high_resolution_clock::duration>(1.0s);
	auto prevInterval = interval;

	int cyl = gui.cylinders;
	int cylIndex = 0;
	float volumes[16] = { 0.50f, 0.92f, 1.0f, 0.72f, 0.71f, 0.51f, 0.92f, 0.68f, 0.73f, 0.72f, 0.77f, 0.95f, 0.63f, 0.62f, 0.59f, 0.56f };

	while (!window.shouldClose())
	{
		window.tick();

		auto now = high_resolution_clock::now();

		float rpm = std::max<float>(1, gui.rpm);
		cyl = gui.cylinders;
		prevInterval = interval;
		interval = duration_cast<high_resolution_clock::duration>(2.0s / (rpm / 60));

		auto timeRemaing = elapse - now;
		auto newTimeRemaining = duration_cast<high_resolution_clock::duration>(timeRemaing * (double(interval.count()) / double(prevInterval.count())));
		elapse -= timeRemaing - newTimeRemaining;
		pitch = rpm / 10000;

		while (elapse < now)
		{
			elapse += interval / cyl;
			cylIndex++;
			cylIndex %= cyl;
			if (gui.rpm < 1) continue;
			//if (cylIndex > cyl - gui.timbre) continue;
			audStream.playSoundAt(SoundEvent(audio.getAudioBuffer(), 0.3 * volumes[cylIndex] + gui.throttlePos * 0.5, 0.8), elapse + 60ms + std::chrono::duration_cast<std::chrono::steady_clock::duration>(450us * volumes[cylIndex] / pitch));
			audStream.playSoundAt(SoundEvent(audio.getAudioBuffer(), 0.6 * volumes[cylIndex] + gui.throttlePos * 0.5, 3.5), elapse + 60ms + std::chrono::duration_cast<std::chrono::steady_clock::duration>(450us * volumes[cylIndex] / pitch));
		}
		std::cout << audStream.underuns << std::endl;
	}
}

void turboDyno()
{
	using namespace espi;
	using namespace std::chrono;
	using namespace std::chrono_literals;
	WindowUtil::audioThreadHint(true);
	Window window(1280, 720, "Engine Music");
	window.makeThisContextCurrent();
	window.setRenderCallBack(render);

	unsigned int source;
	unsigned int buffer;

	alGenSources(1, &source);

	//AudioClip audio("D:/Program Files/FLStudio/Data/Patches/Packs/Drums/Kicks/22in Kick.wav");
	AudioClip audio("rsc/sound/thud.mp3");
	audio.decodeAudio();
	ALUtil::createALBuffers(audio.getAudioBuffer(), &buffer, 1, true);
	alE(alSourcei(source, AL_BUFFER, buffer));

	engmsc::AudioStream audStream(1, true, 44100, 128, 16);

	TurboDynoGUI gui = TurboDynoGUI(window);
	turboGUI = &gui;

	auto elapse = high_resolution_clock::now();
	auto interval = duration_cast<high_resolution_clock::duration>(1.0s);
	auto prevInterval = interval;

	int cyl = gui.cylinders;
	int cylIndex = 0;

	while (!window.shouldClose())
	{
		window.tick();

		auto now = high_resolution_clock::now();

		float rpm = std::max<float>(1, gui.rpm);
		cyl = gui.cylinders;
		prevInterval = interval;
		interval = duration_cast<high_resolution_clock::duration>(2.0s / (rpm / 60));

		auto timeRemaing = elapse - now;
		auto newTimeRemaining = duration_cast<high_resolution_clock::duration>(timeRemaing * (double(interval.count()) / double(prevInterval.count())));
		elapse -= timeRemaing - newTimeRemaining;
		pitch = rpm / 10000;

		while (elapse < now)
		{
			elapse += interval / cyl;
			cylIndex++;
			cylIndex %= cyl;
			if (gui.rpm < 1) continue;
			if (cylIndex > cyl - gui.timbre) continue;
			audStream.playSoundAt(SoundEvent(audio.getAudioBuffer(), 0.3 * (1 - gui.throttlePos) + 0.3), elapse + 60ms);
			audStream.playSoundAt(SoundEvent(audio.getAudioBuffer(), 0.2 + gui.throttlePos * 0.8, 3.5), elapse + 60ms);
		}
		std::cout << audStream.underuns << std::endl;
	}
}

const char* audioFile = "D:/Video/Youtube-DL/mcInterstate.mkv";
bool playSound()
{
	WindowUtil::audioThreadHint(true);
	Window window(1280, 720, "Engine Music");
	window.makeThisContextCurrent();
	window.setRenderCallBack(render);

	AudioClip audio(audioFile);
	//AudioClip audio("rsc/sound/thud.mp3");
	audio.decodeAudio();

	engmsc::AudioStream audStream(2, true, 44100, 128, 16);

	PlaySoundDemoGUI gui = PlaySoundDemoGUI(window);
	theGUI = &gui;

	bool pause = false;
	bool reload = false;

	while (!window.shouldClose())
	{
		window.tick();
		if (gui.pause.isReleased())
		{
			pause = !pause;
			if (pause) audStream.pause();
			else audStream.play();
		}
		if (gui.playSound.isReleasedInside())
		{
			SoundEvent e(audio.getAudioBuffer());
			audStream.playSound(e);
		}
		if (gui.playSound.isReleasedOutside())
		{
			SoundEvent e(audio.getAudioBuffer());
			e.reversed = true;
			audStream.playSound(e);
		}
		if (gui.stop.isReleased())
		{
			audStream.play();
		}
		if (gui.stop.isPressed())
		{
			audStream.stop();
		}
		if (window.getKeyboard().keyJustPressed(KEY_LEFT_CONTROL))
			reload = !reload;
		audStream.masterPitch = gui.pitch.amount * 2.0 - 1.0;
	}
	glfwDestroyWindow(window.getID());
	audStream.close();
	engmsc::BaseGUI::destoryInstance();

	return reload;
}

int main()
{
	dyno();
	//turboDyno();
	

	return 0;
}
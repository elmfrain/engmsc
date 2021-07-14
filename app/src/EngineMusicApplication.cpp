#include <gl/glew.h>
#include <al.h>
#include "elmspi/window/Window.h"
#include "elmspi/gui/GUISlider.h"
#include "elmspi/media/AudioClip.h"
#include "gui/EngMuscGUI.h"
#include <thread>

using namespace engmsc;

Window* theWindow;
BaseGUI* gui;

void render(Window* window)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, window->getWidth(), window->getHeight());
	gui->update(*window);
	gui->render();
}

int main()
{
	using namespace espi;
	WindowUtil::audioThreadHint(true);
	Window window(1280, 720, "Engine Music");
	window.makeThisContextCurrent();
	window.setRenderCallBack(render);

	theWindow = &window;
	unsigned int source;
	unsigned int buffer;
	
	alGenSources(1, &source);
	alGenBuffers(1, &buffer);

	AudioClip audio("rsc/sound/ding.mp3");
	audio.decodeAudio();
	alBufferData(buffer, audio.isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO8, audio.getAudioData(), audio.audioDataSize(), audio.getSampleRate());
	alSourcei(source, AL_BUFFER, buffer);

	gui = new PlaySoundDemoGUI(window);

	while (!window.shouldClose())
	{
		window.tick();
		if (((PlaySoundDemoGUI*)gui)->playSound.justPressed()) alSourcePlay(source);
	}

	return 0;
}
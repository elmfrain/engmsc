#pragma once
#include "elmspi/render/GLUtil.h"
#include "elmspi/window/Window.h"
#include "elmspi/gui/GUISlider.h"
#include "elmspi/render/model/Model.h"
#include "elmspi/maths/Maths.h"
#include "../api/engmscapi.h"

static Font* font = nullptr;

namespace engmsc
{
	class BaseGUI
	{
		protected:
			int windowWidth = 0;
			int windowHeight = 0;
			Window* window;
		public:
			BaseGUI(Window& window)
			{
				if (font == nullptr) font = new Font("C:/Users/Fern/Documents/Git/EngineMusic/rsc/fonts/forced_square.fnt");
				this->window = &window;
			}
			virtual void update(Window& window) { windowWidth = window.getWidth(); windowHeight = window.getHeight(); }
			virtual void render() { GLUtil::setGUIViewPort(0, 0, windowWidth, windowHeight); }
			static void destoryInstance() { font = nullptr; }
	};

	class EngineConfigGUI : public BaseGUI
	{
	private:
		GUISlider cylindersSlider = GUISlider("Cylinders : 1", *font);
		GUISlider timbreSlider = GUISlider("Timbre : 1", *font);
		GUISlider revLimitSlider = GUISlider("Rev Limit: 8000 RPM", *font);

		void updateSliders(Window& window)
		{
			cylindersSlider.amount = (cylinders - 1.0f) / 15.0f;
			revLimitSlider.amount = (revLimit - 3000.0f) / 9000.0f;

			cylindersSlider.setText((std::string("Cylinders: ") + std::to_string(cylinders)).c_str());
			timbreSlider.setText((std::string("Timbre: ") + std::to_string(timbre)).c_str());
			revLimitSlider.setText((std::string("Rev Limit: ") + std::to_string((int)revLimit) + " RPM").c_str());

			cylindersSlider.x = timbreSlider.x = revLimitSlider.x = 100;
			cylindersSlider.width = timbreSlider.width = revLimitSlider.width = window.getWidth() - cylindersSlider.x * 2;
			cylindersSlider.height = timbreSlider.height = revLimitSlider.height = font->fontHeight + 30;
			cylindersSlider.y = window.getHeight() / 4;
			timbreSlider.y = window.getHeight() / 2;
			revLimitSlider.y = window.getHeight() / 4 * 3;
			cylindersSlider.update(window);
			timbreSlider.update(window);
			revLimitSlider.update(window);
			back.update(window);

			cylinders = cylindersSlider.amount * 15 + 1;
			timbre = timbreSlider.amount * 23 + 1;
			revLimit = revLimitSlider.amount * 9000 + 3000;
		}
	public:
		GUIButton back = GUIButton("Back", *font);

		int cylinders = 1;
		int timbre = 1;
		float revLimit = 8000;

		EngineConfigGUI(Window& window) : BaseGUI(window)
		{
			updateSliders(window);
			back.width = 500;
			back.height = font->fontHeight + 30;
		}

		void update(Window& window)
		{
			updateSliders(window);
		}

		void render()
		{
			cylindersSlider.render();
			timbreSlider.render();
			revLimitSlider.render();
			glPushMatrix(); glScalef(0.5f, 0.5f, 1.0f); back.render(); glPopMatrix();
		}
	};

	class EngineDynoGUI : public BaseGUI
	{
		public:
			bool shifting = false;
		protected:
			GLUtil::Framebuffer fbo;
			GUISlider throttle = GUISlider("", *font);
			GUIButton config = GUIButton("Config", *font);
			Model flywheel = Model("rsc/models/flywheel.obj");
			float angle = 0.0f;
			espi::vec2f windowSize = espi::vec2f(0, 0);
			float revLimter = 8000;
			float idle = 750;
			float idleTolerance = 300;
			bool limiterOn = false;
			bool button1InitialPress = false;
			bool button2InitialPress = false;
			bool justChanedGears = false;
			bool upshifted = false;
			bool downshifted = false;
			float targetRpm = 0.0f;
			float gearRatios[8] = { 1.0f, 0.25f, 0.15f, 0.0975f, 0.0683f, 0.0512f, 0.0410f, 0.0345f };

			EngineConfigGUI* configer = nullptr;

			void renderFlywheel(float angleOffset)
			{
				glPushMatrix();
				{
					glTranslatef(windowWidth / 2, windowHeight / 2, 0);
					glScalef(150.0f, 150.f, 1.0f);
					glRotatef(angle + angleOffset, 0, 0, 1);
					glRotatef(90, 1, 0, 0);
					flywheel.renderObject("Flywheel");
				}
				glPopMatrix();
			}
			void updateFlywheel()
			{
				float gearRatio = 1.0f;
				float finalDrive = 2.1f;
				gearRatio = gearRatios[gear] * finalDrive;

				if (justChanedGears && gear > 0)
				{
					targetRpm = 10 * angleSpeed * gearRatios[gear] / gearRatios[prevGear];
					if (targetRpm > revLimter) targetRpm = revLimter;
				}
				if (upshifted && rpm < targetRpm && gear > 0) { upshifted = false; shifting = false; }
				if (downshifted && rpm > targetRpm) { downshifted = false; shifting = false; };
				if (gear > 0 && !upshifted && !downshifted) angleSpeed -= 50.0f * gearRatio * breakPos;

				if (!limiterOn && !upshifted && !downshifted) { angleSpeed += throttle.amount * 30 * gearRatio; throttlePos = throttle.amount; }
				else if (downshifted) { angleSpeed += 30; throttlePos = 1.0f; }
				else throttlePos = 0.0f;

				if (!limiterOn && !downshifted) throttlePos = throttle.amount;

				if ((idle <= angleSpeed * 10.0f) && (angleSpeed * 10.0f <= idle + idleTolerance))
				{
					angleSpeed -= ((10.0f * pow(angleSpeed * 10.0f - idle, 2)) / pow(idleTolerance, 2)) * gearRatio;
				}
				else if (upshifted || downshifted)
				{
					angleSpeed -= 20;
				}
				else
				{
					angleSpeed -= 10 * gearRatio;
				}
				//angleSpeed -= 10.0f;
				angleSpeed = angleSpeed < 0 ? 0 : angleSpeed;
				angle += angleSpeed;
				if (angleSpeed * 10 > revLimter) limiterOn = true;
				if (limiterOn && angleSpeed * 10 < revLimter - 300 * gearRatio) limiterOn = false;
				if (angle > 360) { int a = (360 / (int)angle) * 360; angle -= a; }
				rpm = angleSpeed * 10.0f;
			}
		public:
			float angleSpeed = 0.0f;
			float rpm = 0.0f;
			float throttlePos = 0.0f;
			float breakPos = 0.0f;
			int cylinders = 1;
			int timbre = 1;
			int gear = 0;
			float currentGearRatio = 1.0f;
			int prevGear = 0;
			EngineDynoGUI(Window& window) : BaseGUI(window) 
			{
				throttle.width = 400;
				throttle.height = 200;
				throttle.y = 50;
				throttle.knobWidth = 50;
				config.width = 500;
				config.height = font->fontHeight + 30;
				fbo = GLUtil::Framebuffer(window.getWidth(), window.getHeight(), true, GL_RGBA, GL_FLOAT);
			}
			void update(Window& window) 
			{
				BaseGUI::update(window);
				if (fbo.texture.width != window.getWidth() || fbo.texture.height != window.getHeight())
				{
					fbo.~Framebuffer();
					fbo = GLUtil::Framebuffer(window.getWidth(), window.getHeight(), true, GL_RGBA, GL_FLOAT);
				}
				windowSize.x = window.getWidth();
				windowSize.y = window.getHeight();
				throttle.x = -window.getHeight() + 50;

				justChanedGears = false;
				int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
				if (present)
				{
					int axesCount;
					const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
					breakPos = axes[4] / 2.0f + 0.5f;
					throttle.amount = axes[5] / 2.0f + 0.5f;

					int buttonsCount;
					const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonsCount);
					if (buttons[0] == GLFW_PRESS && button1InitialPress == false && gear < 7)
					{
						button1InitialPress = true;
						prevGear = gear;
						gear++;
						currentGearRatio = gearRatios[gear] * 3;
						justChanedGears = true;
						upshifted = true;
						shifting = true;
					}
					if (buttons[2] == GLFW_PRESS && button2InitialPress == false && gear > 0)
					{
						button2InitialPress = true;
						prevGear = gear;
						gear--;
						currentGearRatio = gearRatios[gear] * 3;
						justChanedGears = true;
						downshifted = true;
						shifting = true;
					}
					if (buttons[0] == GLFW_RELEASE) button1InitialPress = false;
					if (buttons[2] == GLFW_RELEASE) button2InitialPress = false;
				}

				throttle.update(window);
				config.update(window);
				
				updateFlywheel();

				if (config.isReleased())
				{
					configer = new EngineConfigGUI(window);
					configer->cylinders = cylinders;
					configer->timbre = timbre;
					configer->revLimit = revLimter;
				}

				if (configer != nullptr)
				{
					configer->update(window);
					cylinders = configer->cylinders;
					timbre = configer->timbre;
					revLimter = configer->revLimit;
					if (configer->back.isReleased())
					{
						delete configer;
						configer = nullptr;
					}
					throttle.visible = config.visible = false;
				}
				else
				{
					throttle.visible = config.visible = true;
				}
			}
			void render()
			{
				BaseGUI::render();

				if (configer != nullptr) { configer->render(); return; }
				
				glPushMatrix();
				{
					glDisable(GL_BLEND);
					glDisable(GL_ALPHA);
					glDisable(GL_TEXTURE_2D);
					glTranslatef(windowSize.x / 2, windowSize.y / 2, 0);
					glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
					glScalef(150.0f, 150.0f, 1.0f);
					glRotatef(90, 1, 0, 0);
					flywheel.renderObject("Circle");
				}
				glPopMatrix();

				int samples = 100;
				float range = angleSpeed;

				fbo.bind();
				{
					glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					glColor4f(0.0f, 0.0f, 0.0f, 2.0f / samples);
					glEnable(GL_BLEND);
					glEnable(GL_ALPHA);
					glDisable(GL_DEPTH_TEST);
					glBlendFunc(GL_ONE, GL_ONE);
					for (int i = -samples / 2; i < samples / 2; i++)
					{
						renderFlywheel((range / samples) * i);
					}
				}
				fbo.unbind();
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
				fbo.render();

				glPushMatrix();
				{
					glRotatef(-90, 0, 0, 1);
					throttle.render();
				}
				glPopMatrix();
				glColor3f(1.0f, 1.0f, 1.0f);
				font->renderString((std::to_string((int)(angleSpeed * 10)) + " RPM : " + (gear == 0 ? "N" : std::to_string(gear))).c_str());
				glPushMatrix(); glTranslatef(window->getWidth() - config.width / 2, 0.0f, 0.0f); glScalef(0.5f, 0.5f, 1.0f); config.render(); glPopMatrix();
			}
	};

	class TurboDynoGUI : public EngineDynoGUI
	{
		protected:
			engmsc::Engine engine;

			void updateFlywheel()
			{
				throttlePos = throttle.amount;
				engine.throttle(throttlePos);
				engine.tick();
				engine.setExternalTorque(-breakPos * 600.0f);
				angleSpeed = engine.getRPM() / 10.0;
				rpm = engine.getRPM();
				angle += angleSpeed;
			}
		public:
			TurboDynoGUI(Window& window) : EngineDynoGUI(window)
			{

			}
			void update(Window& window)
			{
				BaseGUI::update(window);
				if (fbo.texture.width != window.getWidth() || fbo.texture.height != window.getHeight())
				{
					fbo.~Framebuffer();
					fbo = GLUtil::Framebuffer(window.getWidth(), window.getHeight(), true, GL_RGBA, GL_FLOAT);
				}
				windowSize.x = window.getWidth();
				windowSize.y = window.getHeight();
				throttle.x = -window.getHeight() + 50;

				justChanedGears = false;
				int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
				if (present)
				{
					int axesCount;
					const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
					breakPos = axes[4] / 2.0f + 0.5f;
					throttle.amount = axes[5] / 2.0f + 0.5f;

					int buttonsCount;
					const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonsCount);
					if (buttons[0] == GLFW_PRESS && button1InitialPress == false && gear < 7)
					{
						button1InitialPress = true;
						prevGear = gear;
						gear++;
						currentGearRatio = gearRatios[gear] * 3;
						justChanedGears = true;
						upshifted = true;
						shifting = true;
					}
					if (buttons[2] == GLFW_PRESS && button2InitialPress == false && gear > 0)
					{
						button2InitialPress = true;
						prevGear = gear;
						gear--;
						currentGearRatio = gearRatios[gear] * 3;
						justChanedGears = true;
						downshifted = true;
						shifting = true;
					}
					if (buttons[0] == GLFW_RELEASE) button1InitialPress = false;
					if (buttons[2] == GLFW_RELEASE) button2InitialPress = false;
				}

				throttle.update(window);
				config.update(window);

				updateFlywheel();

				if (config.isReleased())
				{
					configer = new EngineConfigGUI(window);
					configer->cylinders = cylinders;
					configer->timbre = timbre;
					configer->revLimit = revLimter;
				}

				if (configer != nullptr)
				{
					configer->update(window);
					cylinders = configer->cylinders;
					timbre = configer->timbre;
					revLimter = configer->revLimit;
					if (configer->back.isReleased())
					{
						delete configer;
						configer = nullptr;
					}
					throttle.visible = config.visible = false;
				}
				else
				{
					throttle.visible = config.visible = true;
				}
			}
	};

	class PlaySoundDemoGUI : public BaseGUI
	{
		public:
			GUIButton playSound = GUIButton("Play Sound", *font);
			GUIButton pause = GUIButton("Pause", *font);
			GUIButton stop = GUIButton("Stop", *font);
			GUISlider pitch = GUISlider("Pitch", *font);

			PlaySoundDemoGUI(Window& window) : BaseGUI(window)
			{
				playSound.width = 800;
				playSound.height = 150;
				pause.width = 800;
				pause.height = 150;
				stop.width = 800;
				stop.height = 150;
				pitch.width = 800;
				pitch.height = 70;
				pause.x = window.getWidth() / 2 - pause.width / 2;
				pitch.x = pause.x;
				pitch.y = pause.height + 20;
				playSound.x = window.getWidth() / 2 - playSound.width / 2;
				playSound.y = window.getHeight() / 2 - playSound.height / 2;
				stop.x = window.getWidth() / 2 - stop.width / 2;
				stop.y = window.getHeight() - stop.height;
			}
			void update(Window& window)
			{
				BaseGUI::update(window);
				playSound.update(window);
				pause.update(window);
				stop.update(window);
				pitch.update(window);
			}
			void render()
			{
				BaseGUI::render();
				playSound.render();
				pause.render();
				stop.render();
				pitch.render();
			}
	};

	class SpeedSliderGUI : public BaseGUI
	{
		public:
			GUISlider speed = GUISlider("Speed", *font);

			SpeedSliderGUI(Window& window) : BaseGUI(window)
			{
				speed.width = 800;
				speed.height = 150;
				speed.x = window.getWidth() / 2 - speed.width / 2;
				speed.y = window.getHeight() / 2 - speed.height / 2;
			}

			void update(Window& window)
			{
				BaseGUI::update(window);
				speed.update(window);
			}
			void render()
			{
				BaseGUI::render();
				speed.render();
			}
	};
};

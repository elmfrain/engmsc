#pragma once

#include <nanogui/nanogui.h>

#include <engmsc/al/ALAudioContext.hpp>
#include <engmsc/KickProducer.hpp>

class MainScreen : public nanogui::Screen
{
private:
    struct StatusDisplay
    {
        nanogui::TextBox* rpmField;
        nanogui::TextBox* coolantTempField;
        nanogui::TextBox* idleThrottleField;
        nanogui::TextBox* airFuelMassField;
        nanogui::CheckBox* limiterField;
        nanogui::TextBox* nbSoundField;
    };
    struct EngineConfig
    {
        nanogui::Slider* nbCylindersSlider;
    };
    StatusDisplay statusDisplay;
    EngineConfig engineConfig;

    AudioStream audStream;
    ALAudioContext audCtx;
    double elapse;
    double interval = 1.0;
    double prevInterval;
    int cylIndex;
    int nbCyl = 1;
    float volumes[16] = { 0.50f, 0.92f, 1.0f, 0.72f, 0.71f, 0.51f, 0.92f, 0.68f, 0.73f, 0.72f, 0.77f, 0.95f, 0.63f, 0.62f, 0.59f, 0.56f };

    MainScreen();
    void setupGLFWcallbacks();

    int setupEngineStatusWindow(int y);
    int setupEngineInputWindow(int y);
    int setupEngineConfigWindow(int y);
    int setupKickConfigWindow();

public:
    void updateEngineSounds();
    void destroyAudioContext();
    void refreshValues();

    static void setGLFWwindow(GLFWwindow* window);
    static MainScreen* getScreen();
};
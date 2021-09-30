#pragma once

#include <nanogui/nanogui.h>

#include <engmsc/al/ALAudioContext.hpp>
#include <engmsc/KickProducer.hpp>
#include <engmsc-app/ExhaustConfigCanvas.hpp>

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
        nanogui::CheckBox* crankingField;
        nanogui::TextBox* nbSoundField;
    };
    struct EngineConfig
    {
        nanogui::Slider* nbCylindersSlider;
        ExhaustConfigCanvas* exhaustCanvas;
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
    float volumes[16] = { 0.0f };

    MainScreen();
    void setupGLFWcallbacks();

    int setupEngineStatusWindow(int y);
    int setupPowertrainInputWindow(int y);
    int setupEngineConfigWindow(int y);
    int setupKickConfigWindow();
    int setupExhaustOffsetsWindow();

public:
    void updateEngineSounds();
    void destroyAudioContext();
    void refreshValues();

    static void setGLFWwindow(GLFWwindow* window);
    static MainScreen* getScreen();
};
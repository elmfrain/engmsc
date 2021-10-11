#pragma once

#include <nanogui/nanogui.h>

#include <engmsc/al/ALAudioContext.hpp>
#include <engmsc/KickProducer.hpp>
#include <engmsc-app/ExhaustConfigCanvas.hpp>
#include <engmsc/WindProducer.hpp>

class MainScreen : public nanogui::Screen
{
private:
    struct StatusDisplay
    {
        nanogui::TextBox* rpmField;
        nanogui::TextBox* coolantTempField;
        nanogui::TextBox* idleThrottleField;
        nanogui::TextBox* airFuelMassField;
        nanogui::TextBox* torqueField;
        nanogui::CheckBox* limiterField;
        nanogui::CheckBox* crankingField;

        nanogui::TextBox* gearField;
        nanogui::TextBox* gearRatioField;
        nanogui::TextBox* vehicleSpeedField;
    };
    struct EngineConfig
    {
        nanogui::Slider* nbCylindersSlider;
        ExhaustConfigCanvas* exhaustCanvas;
    };
    StatusDisplay statusDisplay;
    EngineConfig engineConfig;

    WindProducer* windProducer;
    AudioStream engineAudioStream;
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
    int setupGearboxStatusWindow();
    int setupExhaustOffsetsWindow();

public:
    void updateEngineSounds();
    void destroyAudioContext();
    void refreshValues();

    static void setGLFWwindow(GLFWwindow* window);
    static MainScreen* getScreen();
};
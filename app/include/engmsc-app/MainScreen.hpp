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
        nanogui::TextBox* nbSoundField;
    };
    StatusDisplay statusDisplay;

    AudioStream audStream;
    ALAudioContext audCtx;

    MainScreen();
    void setupGLFWcallbacks();

    int setupEngineStatusWindow(int y);
    int setupEngineInputWindow(int y);
    int setupEngineConfigWindow(int y);
    int setupKickConfigWindow();

public:
    void destroyAudioContext();
    void refreshValues();

    static void setGLFWwindow(GLFWwindow* window);
    static MainScreen* getScreen();
};
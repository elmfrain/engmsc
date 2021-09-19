#pragma once

#include <nanogui/nanogui.h>

class MainScreen : public nanogui::Screen
{
private:
    struct StatusDisplay
    {
        nanogui::TextBox* rpmField;
        nanogui::TextBox* coolantTempField;
    };
    StatusDisplay statusDisplay;

    MainScreen();
    void setupGLFWcallbacks();

    int setupEngineStatusWindow(int y);
    int setupEngineInputWindow(int y);
    int setupEngineConfigWindow(int y);
public:
    void refreshValues();

    static void setGLFWwindow(GLFWwindow* window);
    static MainScreen* getScreen();
};
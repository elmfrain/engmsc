#include <engmsc-app/MainScreen.hpp>
#include <engmsc-app/FlywheelRenderer.hpp>

#include <GLFW/glfw3.h>

static GLFWwindow* glfwWindow = nullptr;
static MainScreen* screenSingleton = nullptr;

namespace Callbacks
{
    static void onCursorPos(GLFWwindow*, double x, double y)
    {
        MainScreen::getScreen()->cursor_pos_callback_event(x, y);
    }
    static void onMouseButton(GLFWwindow*, int button, int action, int modifiers)
    {
        MainScreen::getScreen()->mouse_button_callback_event(button, action, modifiers);
    }
    static void onKey(GLFWwindow*, int key, int scancode, int action, int mods)
    {
        MainScreen::getScreen()->key_callback_event(key, scancode, action, mods);
    }
    static void onChar(GLFWwindow*, unsigned int codepoint)
    {
        MainScreen::getScreen()->char_callback_event(codepoint);
    }
    static void onDrop(GLFWwindow*, int count, const char** filenames)
    {
        MainScreen::getScreen()->drop_callback_event(count, filenames);
    }
    static void onScroll(GLFWwindow*, double x, double y)
    {
        MainScreen::getScreen()->scroll_callback_event(x, y);
    }
    static void onFramebufferSize(GLFWwindow*, int width, int height)
    {
        MainScreen::getScreen()->resize_callback_event(width, height);
    }
}

MainScreen::MainScreen()
{
    initialize(glfwWindow, false);
    setupGLFWcallbacks();

    setupEngineStatusWindow(0);
    setupEngineInputWindow(100);
    setupEngineConfigWindow(233);

    set_visible(true);
    perform_layout();
}

void MainScreen::setupGLFWcallbacks()
{
    glfwSetCursorPosCallback(glfwWindow, Callbacks::onCursorPos);
    glfwSetMouseButtonCallback(glfwWindow, Callbacks::onMouseButton);
    glfwSetKeyCallback(glfwWindow, Callbacks::onKey);
    glfwSetCharCallback(glfwWindow, Callbacks::onChar);
    glfwSetDropCallback(glfwWindow, Callbacks::onDrop);
    glfwSetScrollCallback(glfwWindow, Callbacks::onScroll);
    glfwSetFramebufferSizeCallback(glfwWindow, Callbacks::onFramebufferSize);
}

int MainScreen::setupEngineStatusWindow(int y)
{
    using namespace nanogui;

    Window* window = new Window(this);
    window->set_title("Engine State");
    window->set_position(Vector2i(0, y));
    window->set_layout(new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 5));
    new Label(window, "RPM");
    statusDisplay.rpmField = new TextBox(window);
    statusDisplay.rpmField->set_value("0");
    statusDisplay.rpmField->set_units("rpm");
    statusDisplay.rpmField->set_fixed_width(120);
    statusDisplay.rpmField->set_alignment(TextBox::Alignment::Left);
    new Label(window, "Coolant Temperature");
    statusDisplay.coolantTempField = new TextBox(window);
    statusDisplay.coolantTempField->set_value("90");
    statusDisplay.coolantTempField->set_units("°C");
    statusDisplay.coolantTempField->set_fixed_width(120);
    statusDisplay.coolantTempField->set_alignment(TextBox::Alignment::Left);
    
    return window->height() + window->position().y();
}

int MainScreen::setupEngineInputWindow(int y)
{
    using namespace nanogui;

    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    Window* window = new Window(this, "Engine Input");
    window->set_position(Vector2i(0, y));
    window->set_layout(new GridLayout(Orientation::Horizontal, 3, Alignment::Middle, 5, 5));

    new Label(window, "Throttle");
    Slider* slider = new Slider(window);
    slider->set_fixed_width(450);
    TextBox* textBox = new TextBox(window, "0");
    textBox->set_units("%");
    textBox->set_fixed_width(75);
    slider->set_callback([textBox, engine](float value)
    {
        textBox->set_value(std::to_string((int) (value * 100)));
        engine->throttle = value;
    });
    slider->set_final_callback([slider, textBox, engine] (float value) 
    {
        slider->set_value(0.0f);
        textBox->set_value("0");
        engine->throttle = 0.0;
    });

    new Label(window, "Brake");
    slider = new Slider(window);
    slider->set_fixed_width(450);
    textBox = new TextBox(window, "0");
    textBox->set_units("%");
    textBox->set_fixed_width(75);
    slider->set_callback([textBox](float value) { textBox->set_value(std::to_string((int) (value * 100))); });
    slider->set_final_callback([slider, textBox] (float value) 
    {
        slider->set_value(0.0f);
        textBox->set_value("0");
    });
    new Label(window, "Clutch");
    slider = new Slider(window);
    slider->set_fixed_width(450);
    textBox = new TextBox(window, "0");
    textBox->set_units("%");
    textBox->set_fixed_width(75);
    slider->set_callback([textBox](float value) { textBox->set_value(std::to_string((int) (value * 100))); });
    slider->set_final_callback([slider, textBox] (float value) 
    {
        slider->set_value(0.0f);
        textBox->set_value("0");
    });

    return window->height() + window->position().y();
}

int MainScreen::setupEngineConfigWindow(int y)
{
    using namespace nanogui;

    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    Window* window = new Window(this, "Engine Configuration");
    window->set_position(Vector2i(0, y));
    window->set_layout(new GridLayout(Orientation::Horizontal, 3, Alignment::Middle, 5, 5));
    new Label(window, "Cylinders");
    Slider* slider = new Slider(window);
    slider->set_fixed_width(160);
    TextBox* textBox = new TextBox(window, "1");
    textBox->set_fixed_width(100);
    textBox->set_alignment(TextBox::Alignment::Left);
    slider->set_callback([slider, textBox](float value) 
    {
        int cylinders = int(value * 15 + 1);
        textBox->set_value(std::to_string(cylinders));
    });
    new Label(window, "Rev Limit");
    slider = new Slider(window);
    slider->set_fixed_width(160);
    slider->set_value(0.591f);
    textBox = new TextBox(window, "7500");
    textBox->set_units("rpm");
    textBox->set_fixed_width(100);
    textBox->set_alignment(TextBox::Alignment::Left);
    slider->set_callback([textBox, engine](float value)
    {
        double v = 11000.0 * value + 1000.0;
        textBox->set_value(std::to_string((int) v));
        engine->revLimiter = v;
    });

    return window->height() + window->position().y();
}

void MainScreen::refreshValues()
{
    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    statusDisplay.rpmField->set_value(std::to_string((int) engine->rpm));
    statusDisplay.coolantTempField->set_value(std::to_string((int) engine->coolantTemperature));
}

void MainScreen::setGLFWwindow(GLFWwindow* window)
{
    glfwWindow = window;
}

MainScreen* MainScreen::getScreen()
{
    if(!screenSingleton)
    {
        if(!glfwWindow)
            throw std::runtime_error("Must set glfw window first!");

        screenSingleton = new MainScreen();
    }
    return screenSingleton;
}
#include <engmsc-app/MainScreen.hpp>
#include <engmsc-app/FlywheelRenderer.hpp>
#include <engmsc-app/ExhaustConfigCanvas.hpp>

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
        if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            FlywheelRenderer::getEngine()->isCranking = true;
        }
        else if(key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
        {
            FlywheelRenderer::getEngine()->isCranking = false;
        }
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

MainScreen::MainScreen() :
    elapse(audStream.getTime()),
    prevInterval(interval)
{
    initialize(glfwWindow, false);
    setupGLFWcallbacks();

    audCtx.initContext();
    audCtx.addStream(audStream);

    setupEngineStatusWindow(0);
    setupEngineInputWindow(200);
    setupEngineConfigWindow(433);
    setupKickConfigWindow();
    setupExhaustOffsetsWindow();

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
    new Label(window, "Idle Valve Steps");
    statusDisplay.idleThrottleField = new TextBox(window);
    statusDisplay.idleThrottleField->set_value("0");
    statusDisplay.idleThrottleField->set_units("steps");
    statusDisplay.idleThrottleField->set_fixed_width(120);
    statusDisplay.idleThrottleField->set_alignment(TextBox::Alignment::Left);
    new Label(window, "Air/Fuel Mass Intake");
    statusDisplay.airFuelMassField = new TextBox(window);
    statusDisplay.airFuelMassField->set_value("0.0");
    statusDisplay.airFuelMassField->set_units("g/s");
    statusDisplay.airFuelMassField->set_fixed_width(120);
    statusDisplay.airFuelMassField->set_alignment(TextBox::Alignment::Left);
    new Label(window, "Limiter");
    statusDisplay.limiterField = new CheckBox(window);
    statusDisplay.limiterField->set_caption("");
    statusDisplay.limiterField->set_checked(false);
    new Label(window, "Cranking");
    statusDisplay.crankingField = new CheckBox(window);
    statusDisplay.crankingField->set_caption("");
    statusDisplay.crankingField->set_checked(false);
    
    return window->height() + window->position().y();
}

int MainScreen::setupEngineInputWindow(int y)
{
    using namespace nanogui;

    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    Window* window = new Window(this, "Engine Input");
    window->set_position(Vector2i(0, 230));
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
    window->set_position(Vector2i(0, 378));
    window->set_layout(new GridLayout(Orientation::Horizontal, 3, Alignment::Middle, 5, 5));
    new Label(window, "Cylinders");
    engineConfig.nbCylindersSlider = new Slider(window);
    engineConfig.nbCylindersSlider->set_fixed_width(160);
    TextBox* textBox = new TextBox(window, "1");
    textBox->set_fixed_width(100);
    textBox->set_alignment(TextBox::Alignment::Left);
    int* o = &nbCyl;
    engineConfig.nbCylindersSlider->set_callback([textBox, o](float value) 
    {
        int cylinders = int(value * 15 + 1);
        textBox->set_value(std::to_string(cylinders));
        *o = cylinders;
    });
    new Label(window, "Rev Limit");
    Slider* slider = new Slider(window);
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
        engine->revLimit = v;
    });

    return window->height() + window->position().y();
}

int MainScreen::setupKickConfigWindow()
{
    using namespace nanogui;

    Window* window = new Window(this, "Kick Configuration");
    window->set_layout(new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 10));
    window->set_position(Vector2i(270, 0));

    new Label(window, "Level");
    Slider* level = new Slider(window);
    level->set_fixed_width(160);
    level->set_value(1.0f);

    new Label(window, "Pitch");
    Slider* pitch = new Slider(window);
    pitch->set_fixed_width(160);

    new Label(window, "EG Attack");
    Slider* attack = new Slider(window);
    attack->set_fixed_width(160);

    new Label(window, "EG Release");
    Slider* release = new Slider(window);
    release->set_fixed_width(160);

    new Label(window, "Mod Amount");
    Slider* modAmount = new Slider(window);
    modAmount->set_fixed_width(160);

    new Label(window, "Mod Rate");
    Slider* modRate = new Slider(window);
    modRate->set_fixed_width(160);

    Button* button = new Button(window, "Kick");
    button->set_callback([=]()
    {
        audStream.playEvent
        (
            SoundEvent(new KickProducer(5.0f, 2.0f))
        );
    });

    statusDisplay.nbSoundField = new TextBox(window);
    statusDisplay.nbSoundField->set_units("s");

    return 0;
}

int MainScreen::setupExhaustOffsetsWindow()
{
    using namespace nanogui;

    Window* window = new Window(this, "Exhaust Offsets");
    window->set_layout(new nanogui::BoxLayout(Orientation::Vertical, Alignment::Middle, 10, 5));
    window->set_position(Vector2i(0, 500));

    ExhaustConfigCanvas* canvas = new ExhaustConfigCanvas(window);
    canvas->set_size(Vector2i(600, 40));

    return 300;
}

#include <algorithm>

void MainScreen::updateEngineSounds()
{
    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    double now = audStream.getTime();
    double rpm = std::max(1.0, engine->rpm);
    prevInterval = interval;
    interval = 2.0 / (rpm / 60.0);

    double timeRemaining = elapse - now;
    double newTimeRemaining = timeRemaining * (interval / prevInterval);
    elapse -= timeRemaining - newTimeRemaining;
    while(elapse < now)
    {
        elapse += interval / nbCyl;
        cylIndex = ++cylIndex % nbCyl;
        if(engine->rpm < 1.0) continue;

        double level = engine->throttle * 0.2 + 0.8;
        KickProducer* p = new KickProducer(!engine->limiterOn ? engine->throttle : 0.0f, std::max(0.0, std::min(rpm / 4000.0, 1.0)));
        audStream.playEventAt(SoundEvent(p, level), elapse + interval * volumes[cylIndex] * 0.1);
    }
}

void MainScreen::destroyAudioContext()
{
    audCtx.destroyContext();
}

void MainScreen::refreshValues()
{
    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();
    char tempString[256] = { 0 };

    statusDisplay.rpmField->set_value(std::to_string((int) engine->rpm));
    statusDisplay.coolantTempField->set_value(std::to_string((int) engine->coolantTemperature));
    statusDisplay.idleThrottleField->set_value(std::to_string(int(engine->idleThrottle * 250)));
    snprintf(tempString, sizeof(tempString), "%.2f", engine->airFuelMassIntake);
    statusDisplay.airFuelMassField->set_value(std::string(tempString));
    statusDisplay.limiterField->set_checked(engine->limiterOn);
    statusDisplay.crankingField->set_checked(engine->isCranking);
    statusDisplay.nbSoundField->set_value(std::to_string(audStream.getTime() - audStream.m_bufferTime));
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
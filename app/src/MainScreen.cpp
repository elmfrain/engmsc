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

        FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();
        FlywheelRenderer::Gearbox* gearbox = FlywheelRenderer::getGearbox();

        if(key == GLFW_KEY_Q && action == GLFW_PRESS) gearbox->setGear(gearbox->gear - 1);
        else if(key == GLFW_KEY_E && action == GLFW_PRESS) gearbox->setGear(gearbox->gear + 1);

        if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            engine->isCranking = true;
        }
        else if(key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
        {
            engine->isCranking = false;
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

    windProducer = new WindProducer();
    audCtx.initContext();
    audCtx.addStream(audStream);
    audStream.playEvent(SoundEvent(windProducer));

    setupEngineStatusWindow(0);
    setupPowertrainInputWindow(280);
    setupExhaustOffsetsWindow();
    setupEngineConfigWindow(433);
    setupGearboxStatusWindow();

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
    new Label(window, "Torque");
    statusDisplay.torqueField = new TextBox(window);
    statusDisplay.torqueField->set_value("0.0");
    statusDisplay.torqueField->set_units("N m");
    statusDisplay.torqueField->set_fixed_width(120);
    statusDisplay.torqueField->set_alignment(TextBox::Alignment::Left);
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

int MainScreen::setupPowertrainInputWindow(int y)
{
    using namespace nanogui;

    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();
    FlywheelRenderer::Gearbox* gearbox = FlywheelRenderer::getGearbox();

    Window* window = new Window(this, "Powertrain Input");
    window->set_position(Vector2i(0, 260));
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
    slider->set_callback([textBox, gearbox](float value)
    {
        textBox->set_value(std::to_string((int) (value * 100)));
        gearbox->brakeAmount = value;
    });
    slider->set_final_callback([slider, textBox, gearbox] (float value) 
    {
        slider->set_value(0.0f);
        textBox->set_value("0");
        gearbox->brakeAmount = 0.0;
    });
    new Label(window, "Clutch");
    slider = new Slider(window);
    slider->set_fixed_width(450);
    textBox = new TextBox(window, "0");
    textBox->set_units("%");
    textBox->set_fixed_width(75);
    slider->set_callback([textBox, gearbox](float value)
    {
        textBox->set_value(std::to_string((int) (value * 100)));
        gearbox->clutchAmount = value;
    });
    slider->set_final_callback([slider, textBox, gearbox] (float value) 
    {
        slider->set_value(0.0f);
        textBox->set_value("0");
        gearbox->clutchAmount = 0.0;
    });

    return window->height() + window->position().y();
}

int MainScreen::setupEngineConfigWindow(int y)
{
    using namespace nanogui;

    ExhaustConfigCanvas* exhaustCanvas = engineConfig.exhaustCanvas;
    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    Window* window = new Window(this, "Engine Configuration");
    window->set_position(Vector2i(0, 610));
    window->set_layout(new GridLayout(Orientation::Horizontal, 3, Alignment::Middle, 5, 5));
    new Label(window, "Cylinders");
    engineConfig.nbCylindersSlider = new Slider(window);
    engineConfig.nbCylindersSlider->set_fixed_width(160);
    TextBox* textBox = new TextBox(window, "1");
    textBox->set_fixed_width(100);
    textBox->set_alignment(TextBox::Alignment::Left);
    int* o = &nbCyl;
    engineConfig.nbCylindersSlider->set_callback([textBox, o, exhaustCanvas](float value) 
    {
        int cylinders = int(value * 15 + 1);
        textBox->set_value(std::to_string(cylinders));
        exhaustCanvas->setNbCylinders(cylinders);
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

int MainScreen::setupGearboxStatusWindow()
{
    using namespace nanogui;

    Window* window = new Window(this, "Gearbox State");
    window->set_layout(new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 5));
    window->set_position(Vector2i(270, 0));

    new Label(window, "Gear");
    statusDisplay.gearField = new TextBox(window);
    statusDisplay.gearField->set_value("N");
    statusDisplay.gearField->set_fixed_width(120);
    statusDisplay.gearField->set_alignment(TextBox::Alignment::Left);

    new Label(window, "Gear Ratio");
    statusDisplay.gearRatioField = new TextBox(window);
    statusDisplay.gearRatioField->set_value("--");
    statusDisplay.gearRatioField->set_fixed_width(120);
    statusDisplay.gearRatioField->set_alignment(TextBox::Alignment::Left);

    new Label(window, "Vehicle Speed");
    statusDisplay.vehicleSpeedField = new TextBox(window);
    statusDisplay.vehicleSpeedField->set_value("0.0");
    statusDisplay.vehicleSpeedField->set_fixed_width(120);
    statusDisplay.vehicleSpeedField->set_units("km/h");
    statusDisplay.vehicleSpeedField->set_alignment(TextBox::Alignment::Left);

    return 0;
}

int MainScreen::setupExhaustOffsetsWindow()
{
    using namespace nanogui;

    Window* window = new Window(this, "Exhaust Offsets");
    window->set_layout(new BoxLayout(Orientation::Vertical, Alignment::Minimum, 10, 5));
    window->set_position(Vector2i(0, 404));

    ComboBox* cBox = new ComboBox(window,
    {
        "Default Preset",
        "2Cyl: Harley-Davidson V-Twin",
        "3Cyl: Suzuki Cappuccino",
        "4Cyl: Honda Civic",
        "4Cyl: Subaru Impreza WRX STi",
        "5Cyl: Audi Quattro",
        "6Cyl: Toyota Supra",
        "6Cyl: Nissan GTR R35",
        "8Cyl: Ferrari F40",
        "8Cyl: Chevrolet Corvette C7"
    });

    engineConfig.exhaustCanvas = new ExhaustConfigCanvas(window);
    engineConfig.exhaustCanvas->set_size(Vector2i(480, 40));
    engineConfig.exhaustCanvas->setOffsets(volumes, 16);

    TextBox* textBox = new TextBox(window);
    textBox->set_value("1");
    textBox->set_units("cyl#");
    textBox->set_alignment(TextBox::Alignment::Right);
    textBox->set_fixed_width(100);

    Label* label = new Label(window, "Offset: 1.00");
    label->set_fixed_width(100);

    Slider* offsetSlider = new Slider(window);
    offsetSlider->set_fixed_width(480);
    offsetSlider->set_value(0.5f);
    float* offsets = volumes;
    ExhaustConfigCanvas* canvas = engineConfig.exhaustCanvas;
    offsetSlider->set_callback([offsets, canvas, label] (float value) 
    {
        int cylIndex = canvas->getSelectedCylinder() - 1;
        offsets[cylIndex] = 2.0f * value - 1.0f;
        canvas->setOffsets(offsets, 16);

        char string[16] = {0};
        snprintf(string, sizeof(string), "Offset: %.2f", offsets[cylIndex]);
        label->set_caption(string);
    });

    engineConfig.exhaustCanvas->setCallback([offsets, offsetSlider, canvas, label, textBox] ()
    {
        int cylIndex = canvas->getSelectedCylinder() - 1;
        char string[16] = {0};
        snprintf(string, sizeof(string), "Offset: %.2f", offsets[cylIndex]);
        label->set_caption(string);

        offsetSlider->set_value(offsets[cylIndex] * 0.5f + 0.5f);
        textBox->set_value(std::to_string(canvas->getSelectedCylinder()));
    });

    cBox->set_fixed_width(480);
    cBox->set_callback([offsets, canvas] (int preset)
    {
        float thePreset[16] = { 0.0f };
        #define setPreset(...) { float p[] = __VA_ARGS__; memcpy(thePreset, p, sizeof(p)); }

        switch (preset)
        {
        case 1:
            setPreset({ 0.0f, 0.5f });
            break;
        case 2:
            setPreset({ 0.0f, 0.16f, 0.16f });
            break;
        case 3:
            setPreset({ 0.0f, 0.09f, 0.1f, 0.06f });
            break;
        case 4:
            setPreset({ 0.0f, 0.09f, 0.42f, 0.34f });
            break;
        case 5:
            setPreset({ 0.0f, 0.05f, 0.12f, 0.14f, 0.17f });
            break;
        case 6:
            setPreset({ 0.0f, 0.16f, 0.21f, 0.19f, 0.16f, 0.12f });
            break;
        case 7:
            setPreset({ 0.0f, 0.0f, 0.0f, 0.25f, 0.25f, 0.25f });
            break;
        case 8:
            setPreset({ 0.0f, 0.01f, 0.04f, 0.07f, 0.21f, 0.22f, 0.19f, 0.20f });
            break;
        case 9:
            setPreset({ 0.0f, -0.02f, 0.57f, 0.0f, 0.23f, 0.22f, 0.61f, 0.20f });
            break;
        }

        memcpy(offsets, thePreset, sizeof(thePreset));
        canvas->setOffsets(thePreset, 16);
    });

    return 300;
}

#include <algorithm>

Iir::Butterworth::LowPass<4> throttleLowpass;

void MainScreen::updateEngineSounds()
{
    throttleLowpass.setup(60.0, 8.0);

    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();

    double now = audStream.getTime();
    double rpm = std::max(1.0, engine->rpm);
    prevInterval = interval;
    interval = 2.0 / (rpm / 60.0);

    double throttleSoundLevel = !engine->limiterOn ? engine->throttle * 2.2f + 0.8f: 0.5f;
    throttleSoundLevel = throttleLowpass.filter(throttleSoundLevel);

    double timeRemaining = elapse - now;
    double newTimeRemaining = timeRemaining * (interval / prevInterval);
    elapse -= timeRemaining - newTimeRemaining;
    while(elapse < now)
    {
        elapse += interval / nbCyl;
        cylIndex = ++cylIndex % nbCyl;
        if(engine->rpm < 1.0) continue;

        
        KickProducer* p = new KickProducer(throttleSoundLevel, std::max(0.0, std::min(rpm / 4000.0, 1.0)));
        //KickProducer* p = new KickProducer(6.0f, std::max(0.0, std::min(rpm / 4000.0, 1.0)));
        audStream.playEventAt(SoundEvent(p), elapse + 0.03 + (interval / nbCyl) * 0.5f * volumes[cylIndex]);
    }

    windProducer->setWindVelocity(FlywheelRenderer::getGearbox()->kmh);
}

void MainScreen::destroyAudioContext()
{
    audCtx.destroyContext();
}

void MainScreen::refreshValues()
{
    FlywheelRenderer::Engine* engine = FlywheelRenderer::getEngine();
    FlywheelRenderer::Gearbox* gearbox = FlywheelRenderer::getGearbox();
    #define formatString(format, ...) { memset(tempString, 0, sizeof(tempString)); snprintf(tempString, sizeof(tempString), format, __VA_ARGS__); }
    char tempString[256];

    statusDisplay.rpmField->set_value(std::to_string((int) engine->rpm));
    statusDisplay.coolantTempField->set_value(std::to_string((int) engine->coolantTemperature));
    statusDisplay.idleThrottleField->set_value(std::to_string(int(engine->idleThrottle * 250)));
    formatString("%.2f", engine->airFuelMassIntake);
    statusDisplay.airFuelMassField->set_value(tempString);
    statusDisplay.limiterField->set_checked(engine->limiterOn);
    statusDisplay.crankingField->set_checked(engine->isCranking);
    formatString("%.2f", engine->torque);
    statusDisplay.torqueField->set_value(tempString);

    statusDisplay.gearField->set_value(gearbox->gear == 0 ? "N" : std::to_string(gearbox->gear));
    if(gearbox->gear > 0) formatString("%.2f", gearbox->gearRatios[gearbox->gear - 1]);
    statusDisplay.gearRatioField->set_value(gearbox->gear == 0 ? "--" : tempString);
    formatString("%.1f", gearbox->kmh);
    statusDisplay.vehicleSpeedField->set_value(tempString);
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
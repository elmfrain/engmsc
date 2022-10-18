#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"
#include "engmsc/Engine2DRenderer.hpp"
#include "engmsc/Gauge.hpp"
#include "engmsc/Engine.hpp"
#include "engmsc/EnginePhysics.hpp"
#include "AudioContext.hpp"
#include "engmsc/EngineAudio.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <Iir.h>

EMLogger mainLogger("Main");

void clearGLErrors()
{
    GLenum error;
    do
    {
        error = glGetError();

        if(error != GL_NO_ERROR)
        {
            mainLogger.submodule("OpenGL").errorf("ID: %d", error);
        }
    } while (error != GL_NO_ERROR);
    
}

class LowPassFilter : public EMAudioFilter
{
private:
    Iir::Butterworth::LowPass<4> lowpass;
public:
    LowPassFilter()
    {
        lowpass.setup(EMSAMPLE_RATE, 1000);
    }

    virtual void filter(float* dest, float* in, size_t len) override
    {
        for(size_t i = 0; i < len; i++)
        {
            dest[i] = lowpass.filter(in[i]);
        }
    }
};

class HighPassFilter : public EMAudioFilter
{
private:
    Iir::Butterworth::HighPass<4> highpass;
public:
    HighPassFilter()
    {
        highpass.setup(EMSAMPLE_RATE, 500);
    }

    virtual void filter(float* dest, float* in, size_t len) override
    {
        for(size_t i = 0; i < len; i++)
        {
            dest[i] = highpass.filter(in[i]);
        }
    }
};

int main(int argc, char* argv[])
{
    mainLogger.infof("Starting Up");

    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_RED_BITS, 16);
    glfwWindowHint(GLFW_GREEN_BITS, 16);
    glfwWindowHint(GLFW_BLUE_BITS, 16);

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    const EMKeyboard& keyboard = window.getKeyboard();
    const EMMouse& mouse = window.getMouse();
    emui::setWindow(window);

    // Create gauges
    typedef std::shared_ptr<EMGauge> Gauge;
    std::vector<Gauge> gauges;
    gauges.push_back(std::make_shared<EMGauge>()); // RPM Gauge
    gauges.back()->setText("RPM x1000");
    gauges.back()->setRange(0, 8);
    gauges.back()->getProfile().subdivisions = 3;
    gauges.push_back(std::make_shared<EMGauge>()); // Cyl Baro Gauge
    gauges.back()->setText("Cyl kPa");
    gauges.back()->setRange(0, 800);
    gauges.back()->getProfile().subdivisions = 3;
    gauges.push_back(std::make_shared<EMGauge>()); // Cyl Gas Mass Gauge
    gauges.back()->setText("Cyl Gas kg");
    gauges.back()->setRange(0, 2);
    gauges.back()->getProfile().precision = 1;
    gauges.back()->getProfile().subdivisions = 3;
    gauges.push_back(std::make_shared<EMGauge>());
    gauges.back()->setText("Exhaust Gas Kg");
    gauges.back()->setRange(0, 2);
    gauges.back()->getProfile().precision = 1;
    gauges.back()->getProfile().subdivisions = 3;
    gauges.push_back(std::make_shared<EMGauge>());
    gauges.back()->setText("Exhaust kPa");
    gauges.back()->setRange(50, 150);
    gauges.back()->getProfile().subdivisions = 3;
    for(Gauge& gauge : gauges)
    {
        gauge->getProfile().radius = 120;
    }

    // Create Engine Assembly
    EMEngineAssembly engine;
    engine.frictionTorque = 100000;
    engine.rotationalMass = 800;
    engine.cylinders.back().deckClearance = 0.1f;
    //engine.cylinders.back().bankAngle = -0.785f;
    engine.cylinders.emplace_back();
    engine.cylinders.back().deckClearance = 0.1f;
    engine.cylinders.back().angleOffset = 2.094f;
    engine.cylinders.back().exhaustRunnerLength = 1.2f;
    engine.cylinders.back().exhaustRunnerVolume = 0.32f;
    //engine.cylinders.back().bankAngle = 0.785f;
    engine.cylinders.emplace_back();
    engine.cylinders.back().deckClearance = 0.1f;
    engine.cylinders.back().angleOffset = 4.189f;
    engine.cylinders.back().exhaustRunnerLength = 0.8f;
    engine.cylinders.back().exhaustRunnerVolume = 0.24f;
    engine.cylinders.emplace_back();
    engine.cylinders.back().deckClearance = 0.1f;
    engine.cylinders.back().angleOffset = 10.472f;
    engine.cylinders.back().exhaustRunnerLength = 1.4f;
    engine.cylinders.back().exhaustRunnerVolume = 0.36f;
    engine.cylinders.emplace_back();
    engine.cylinders.back().deckClearance = 0.1f;
    engine.cylinders.back().angleOffset = 8.378f;
    engine.cylinders.back().exhaustRunnerLength = 1.0f;
    engine.cylinders.back().exhaustRunnerVolume = 0.28f;
    engine.cylinders.emplace_back();
    engine.cylinders.back().deckClearance = 0.1f;
    engine.cylinders.back().angleOffset = 6.283f;
    engine.cylinders.back().exhaustRunnerLength = 1.6f;
    engine.cylinders.back().exhaustRunnerVolume = 0.40f;

    // Init engine renderer and physics
    EMEngine2DRenderer::init();
    EMEngine2DRenderer::setEngineAssembly(engine);
    EMEnginePhysics::setEngineAssembly(engine);

    glEnable(GL_MULTISAMPLE);

    // Setup Audio
    EMOpenALContext audioCtx;
    audioCtx.initContext();
    EMAudioStream audStream;
    audioCtx.addStream(audStream);
    audStream.newInsert().addFilter(new LowPassFilter());
    audStream.getInsert(1).addFilter(new HighPassFilter());
    //audStream.getMainInsert().addFilter(new LowPassFilter());
    audStream.play(EMAudioEvent(EMEngineAudio::getAudioProducer()), 1);

    EMTimer timer(40000.0);
    EMTimer idleTimer(9.0);
    bool limiter = false;
    double idle = 0.0;
    double idleD = 0.0;

    while(!window.shouldClose())
    {
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        audioCtx.update();

        int count;
        const float* axis = glfwGetJoystickAxes(0, &count);

        // Render engine
        double torque = 0.0;
        if(keyboard.isKeyPressed(GLFW_KEY_LEFT)) torque = -250000;
        if(keyboard.isKeyPressed(GLFW_KEY_RIGHT)) torque = 250000;
        
        idle = glm::max(0.0, glm::min(idle, 1.0));
        engine.trottle = ((limiter ? -1.0f : axis[5]) + 1.0f) * 4.0 + 0.05;
        engine.trottle += idle;

        torque += ((limiter ? -1.0f : axis[5]) + 1.0f) * 250000 + idle * 500000;
        torque -= 0.0 < engine.crankSpeed ? ((limiter ? 0.5f : axis[4]) + 1.0f) * 250000 : 0.0;
        int steps = timer.ticksPassed();
        for(int i = 0; i < steps; i++)
        {
            EMEnginePhysics::applyTorque(torque);
            EMEnginePhysics::update(1.0 / timer.getTPS(), 1);
            //EMEnginePhysics::update(0.009 / timer.getTPS(), 1);
        }
        EMEngine2DRenderer::render();

        // UI Rendering
        emui::setupUIRendering();

        // Render gauges
        float rpm = 60.0f * float(engine.crankSpeed / glm::two_pi<double>());
        gauges[0]->setValue(rpm / 1000);
        gauges[1]->setValue((float) EMEnginePhysics::getCylPressure(0) * 1e-3f);
        gauges[2]->setValue((float) EMEnginePhysics::getCylGasMass(0));
        gauges[3]->setValue((float) EMEnginePhysics::getExhaustGasMass(0));
        gauges[4]->setValue((float) EMEnginePhysics::getExhaustPressure(0) * 1e-3f);
        float gaugex = emui::getUIWidth();
        for(Gauge& gauge : gauges)
        {
            float radius = gauge->getProfile().radius;
            gaugex -= radius;
            gauge->x = gaugex;
            gauge->y = emui::getUIHeight() - radius;
            gauge->draw();
            gaugex -= radius;
        }

        if(7000 < rpm) limiter = true;
        else if(limiter && 6800 > rpm) limiter = false;


        if(idleTimer.ticksPassed())
        {
            double targetRPM = 1000;
            if(400 < rpm)
            {
                idleD += targetRPM - rpm;
                idleD = glm::max(-500.0, glm::min(idleD, 500.0));
                idle = (targetRPM - rpm) * 0.0022;
                idle += idleD * 0.001;
            }
            else idle = 0.0;
        }

        emui::genString(std::to_string(idle).c_str(), 0, 0, 0xFFFFFFFF, emui::TOP_LEFT);
        emui::genString(std::to_string(EMEngineAudio::m_engineLog.size()).c_str(), 300, 0, 0xFFFFFFFF, emui::TOP_LEFT);

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    audioCtx.destoryContext();
    glfwTerminate();

    return 0;
}
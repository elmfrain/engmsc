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
    gauges.push_back(std::make_shared<EMGauge>()); // Intake Baro Gauge
    gauges.back()->setText("Exhaust kPa");
    gauges.back()->setRange(0, 400);
    gauges.back()->getProfile().subdivisions = 3;
    for(Gauge& gauge : gauges)
    {
        gauge->getProfile().radius = 120;
    }

    // Create Engine Assembly
    EMEngineAssembly engine;
    engine.frictionTorque = 5000;
    engine.rotationalMass = 1000;
    engine.cylinders.back().deckClearance = 0.1f;
    //engine.cylinders.back().angleOffset = glm::half_pi<double>();

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
    audStream.play(EMAudioEvent(EMEngineAudio::getAudioProducer()));

    EMTimer timer(6000.0);

    while(!window.shouldClose())
    {
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        audioCtx.update();

        // Render engine
        double torque = 0.0;
        if(keyboard.isKeyPressed(GLFW_KEY_LEFT)) torque = -50000;
        if(keyboard.isKeyPressed(GLFW_KEY_RIGHT)) torque = 50000;
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
        gauges[3]->setValue((float) EMEnginePhysics::getIntakePressure(0) * 1e-3f);
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

        emui::genString(std::to_string(EMEnginePhysics::getCylPressure(0)).c_str(), 0, 0, 0xFFFFFFFF, emui::TOP_LEFT);
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
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

class Prodtest : public EMAudioProducer
{
private:
    double pos = 0.0;
public:
    size_t placeSamples(float* buffer, size_t bufferLen) override
    {
        if(pos / EMSAMPLE_RATE > getDuration())
        {
            return 0;
        }

        for(size_t i = 0; i < bufferLen; i++)
        {
            buffer[i] += (float) glm::sin(pos * 3.14 * 2000 * m_pitch / EMSAMPLE_RATE) * m_gain;
            pos++;
        }

        return 0;
    }
    double getDuration() const override
    {
        return 0.25;
    }
    bool hasExpired() const override
    {
        return pos / EMSAMPLE_RATE > getDuration();
    }
};

int main(int argc, char* argv[])
{
    mainLogger.infof("Starting Up");

    glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    const EMKeyboard& keyboard = window.getKeyboard();
    const EMMouse& mouse = window.getMouse();
    emui::setWindow(window);

    // Create gauges
    EMGauge tach;
    tach.setText("RPMx1000");
    EMGauge::Profile& tachProfile = tach.getProfile();
    tachProfile.radius = 120;
    tachProfile.numMarkings = 5;
    tachProfile.subdivisions = 5;
    tach.applyProfile();
    tach.setRange(0, 4);
    EMGauge baro;
    baro.setText("Cyl kPa");
    EMGauge::Profile& baroProfile = baro.getProfile();
    baroProfile.radius = 120;
    baroProfile.numMarkings = 9;
    baroProfile.subdivisions = 3;
    baro.applyProfile();
    baro.setRange(0, 800);

    EMTimer timer(1500.0);

    // Create Engine Assembly
    EMEngineAssembly engine;
    engine.frictionTorque = 1000;
    engine.rotationalMass = 1000;
    engine.cylinders.back().pistonFrictionForce = 200;
    engine.cylinders.back().deckClearance = 0.1f;

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

    while(!window.shouldClose())
    {
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(mouse.buttonJustPressed(GLFW_MOUSE_BUTTON_1))
        {
            audStream.play(EMAudioEvent(new Prodtest(), 0.3f, 0.5f * float(rand()) / RAND_MAX + 0.5f));
        }
        audioCtx.update();

        // Render engine
        double torque = 0.0;
        if(keyboard.isKeyPressed(GLFW_KEY_LEFT)) torque = -30000;
        if(keyboard.isKeyPressed(GLFW_KEY_RIGHT)) torque = 30000;
        int steps = timer.ticksPassed();
        for(int i = 0; i < steps; i++)
        {
            EMEnginePhysics::applyTorque(torque);
            EMEnginePhysics::update(1.0 / timer.getTPS(), 1);
        }
        EMEngine2DRenderer::render();

        // UI Rendering
        emui::setupUIRendering();

        float rpm = 60.0f * float(engine.crankSpeed / glm::two_pi<double>());
        tach.x = window.getWidth() - 120.0f;
        tach.y = baro.y = window.getHeight() - 120.0f;
        tach.setValue(rpm / 1000.0f);
        tach.draw();
        baro.x = tach.x - 240;
        baro.setValue((float) EMEnginePhysics::getCylPressure(0) * 1e-3f);
        baro.draw();

        emui::genString(std::to_string(audStream.getNbEvents()).c_str(), 0, 0, 0xFFFFFFFF, emui::TOP_LEFT);

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    audioCtx.destoryContext();
    glfwTerminate();

    return 0;
}
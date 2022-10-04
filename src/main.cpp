#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"
#include "engmsc/Engine2DRenderer.hpp"
#include "engmsc/Gauge.hpp"
#include "engmsc/Engine.hpp"

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

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    const EMKeyboard& keyboard = window.getKeyboard();
    const EMMouse& mouse = window.getMouse();
    emui::setWindow(window);

    // Create tachometer
    EMGauge tach;
    tach.setText("RPMx1000");
    EMGauge::Profile& tachProfile = tach.getProfile();
    tachProfile.radius = 150;
    tachProfile.numMarkings = 5;
    tachProfile.subdivisions = 5;
    tach.applyProfile();
    tach.setRange(0, 4);

    // Create Engine Assembly
    EMEngineAssembly engine;

    // Init engine renderer
    EMEngine2DRenderer::init();
    EMEngine2DRenderer::setEngineAssembly(engine);

    double prevTime = glfwGetTime();
    glEnable(GL_MULTISAMPLE);

    while(!window.shouldClose())
    {
        double time = glfwGetTime();
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render engine
        EMEngineCylinder cyl = engine.cylinders.back();
        double b = -engine.crankAngle + glm::half_pi<double>();
        double pistonY = 
        glm::sin(glm::acos(glm::cos(b) * cyl.stroke / (2.0 * cyl.rodLength))) * cyl.rodLength
        + glm::sin(b) * cyl.stroke / 2.0;
        double conrodAngle = glm::asin(glm::sin(engine.crankAngle) / cyl.rodLength);
        double mass = 40.0;
        double torque = 0.0;
        double pistonForce = 0.0;
        if(keyboard.isKeyPressed(GLFW_KEY_LEFT)) torque = -10;
        if(keyboard.isKeyPressed(GLFW_KEY_RIGHT)) torque = 10;
        if(keyboard.isKeyPressed(GLFW_KEY_DOWN)) pistonForce = 24;
        double frictionTorque = 0.0 < engine.crankSpeed ? 3.0 : -3.0;
        torque += pistonForce * glm::sin(conrodAngle) * pistonY;
        if(-0.1 < engine.crankSpeed && engine.crankSpeed < 0.1)
        {
            engine.crankSpeed = 0.0;
            frictionTorque = 0.0;
        }
        double accel = (torque - frictionTorque) / mass;
        engine.crankSpeed += accel;
        engine.crankAngle += engine.crankSpeed * (time - prevTime);
        prevTime = time;
        EMEngine2DRenderer::render();

        // UI Rendering
        emui::setupUIRendering();

        float rpm = 60.0f * float(engine.crankSpeed / glm::two_pi<double>());
        tach.x = window.getWidth() - 150.0f;
        tach.y = window.getHeight() - 150.0f;
        tach.setValue(rpm / 1000.0f);
        tach.draw();

        emui::genString(std::to_string(rpm).c_str(), 0, 0, 0xFFFFFFFF, emui::TOP_LEFT);

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
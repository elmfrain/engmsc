#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"
#include "engmsc/Gauge.hpp"

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

    glEnable(GL_MULTISAMPLE);

    EMGauge gauge;
    gauge.setText("§lRPMx1000");

    while(!window.shouldClose())
    {
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        if(keyboard.isKeyPressed(GLFW_KEY_SPACE)) 
            glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        emui::setupUIRendering();

        gauge.x = window.getWidth() / 2.0f;
        gauge.y = window.getHeight() / 2.0f;
        gauge.setValue(mouse.cursorX() / 900.0f - 0.1f);

        gauge.draw();

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
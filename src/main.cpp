#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"

#include <glm/gtx/transform.hpp>

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

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    const EMKeyboard& keyboard = window.getKeyboard();
    const EMMouse& mouse = window.getMouse();
    emui::setWindow(window);

    glEnable(GL_MULTISAMPLE);

    float y = 0;

    while(!window.shouldClose())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        if(keyboard.isKeyPressed(GLFW_KEY_SPACE)) 
            glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        if(mouse.justScrolled())
        {
            y += mouse.scrollDeltaY() * 10;
        }

        emui::setupUIRendering();

        emui::genQuad(100, 100, 200, 200, -1);
        emui::genHorizontalLine(500, 50, 1000, 0xFF000000, 2);
        emui::genLine(300, y, mouse.cursorX(), mouse.cursorY(),
        mouse.isButtonPressed(GLFW_MOUSE_BUTTON_1) ? 0xFFFF0000 : 0xFF00FF00, 5);
        emui::genString("Test String", 0, 0, -1, 0);

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
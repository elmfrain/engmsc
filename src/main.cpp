#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"
#include "Button.hpp"

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

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    const EMKeyboard& keyboard = window.getKeyboard();
    const EMMouse& mouse = window.getMouse();
    EMButton test("Button");
    test.x = test.y = 500;
    emui::setWindow(window);

    glEnable(GL_MULTISAMPLE);

    float y = 0;

    while(!window.shouldClose())
    {
        glViewport(0, 0, window.getWidth(), window.getHeight());
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

        char output[1024];
        snprintf(output, 1023, "§n§l%.2f §r%.2f 0xFF555555" ,mouse.cursorX(), mouse.cursorY());
        emui::genString(output, mouse.cursorX(), mouse.cursorY(), -1, emui::CENTER);

        test.draw();
        if(test.justPressed())
        {
            mainLogger.infof("Clicked From Test Button");
        }

        emui::renderBatch();

        clearGLErrors();
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
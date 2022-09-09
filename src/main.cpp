#include <iostream>

#include "Window.hpp"
#include "Logger.hpp"
#include "UIRender.hpp"
#include "GLInclude.hpp"

#include <glm/gtx/transform.hpp>

EMLogger mainLogger("Main");

int main(int argc, char* argv[])
{
    mainLogger.infof("Starting Up");

    glfwInit();

    EMWindow window(1280, 720, "Engmsc by Elmfer");
    emui::setWindow(window);

    while(!window.shouldClose())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        emui::setupUIRendering();
        emui::genQuad(100, 100, 200, 200, -1);
        emui::genHorizontalLine(500, 50, 1000, 0xFF000000, 2);
        emui::genLine(300, 300, 600, 0, 0xFFFF0000, 5);
        emui::renderBatch();

        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
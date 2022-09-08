#include <iostream>

#include "EMWindow.hpp"
#include "GLInclude.hpp"

int main(int argc, char* argv[])
{
    glfwInit();

    EMWindow window(1280, 720, "Engmsc by Elmfer");

    while(!window.shouldClose())
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
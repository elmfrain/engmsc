#include <iostream>

#include <GLFW/glfw3.h>

#include "EMWindow.hpp"

int main(int argc, char* argv[])
{
    glfwInit();

    EMWindow window(1280, 720, "Engmsc by Elmfer");

    while(!window.shouldClose())
    {
        window.swapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
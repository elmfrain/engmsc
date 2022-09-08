#include <iostream>

#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Engmsc - Elmfer", NULL, NULL);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}
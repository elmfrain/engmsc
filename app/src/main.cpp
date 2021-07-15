#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nanogui/nanogui.h>

#include <SFML/Audio.hpp>
#include <iostream>

sf::SoundBuffer defaultSound;
sf::Sound sound;

namespace ngi = nanogui;

GLFWwindow* glfwWindow = 0;

void initializeGLFWwindow();

int main()
{
    defaultSound.loadFromFile("rsc/sound/thud.wav");
    sound.setBuffer(defaultSound);

    initializeGLFWwindow();

    while(!glfwWindowShouldClose(glfwWindow))
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);        

        glfwPollEvents();
        glfwSwapBuffers(glfwWindow);
    }

    glfwTerminate();
    return 0;
}

void initializeGLFWwindow()
{
    glfwInit();

    glfwSetTime(0);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    glfwWindow = glfwCreateWindow(1280, 720, "EngMsc Application", NULL, NULL);
    if(!glfwWindow)
    {
        throw std::runtime_error("Unable to initialize GLFW window!");
    }

    glfwMakeContextCurrent(glfwWindow);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        throw std::runtime_error("Unable to initialize GLAD!");
    }
}
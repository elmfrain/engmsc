#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <engmsc-app/MainScreen.hpp>
#include <engmsc-app/FlywheelRenderer.hpp>

#include <engmsc/al/ALAudioContext.hpp>

#include <iostream>

GLFWwindow* glfwWindow = 0;
MainScreen* mainScreen = nullptr;

void initializeGLFWwindow();

int main()
{
    initializeGLFWwindow();

    MainScreen::setGLFWwindow(glfwWindow);
    mainScreen = MainScreen::getScreen();

    FlywheelRenderer::initialize();
    AudioStream stream;
    ALAudioContext ctx;
    ctx.initContext();
    ctx.addStream(stream);

    while(!glfwWindowShouldClose(glfwWindow))
    {
        glClearColor(0.35f, 0.5f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);        
        FlywheelRenderer::draw();
        mainScreen->refreshValues();
        mainScreen->draw_widgets();

        glfwPollEvents();
        glfwSwapBuffers(glfwWindow);
    }

    ctx.destroyContext();
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

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    glfwWindow = glfwCreateWindow(1280, 720, "EngMsc Application", NULL, NULL);
    if(!glfwWindow)
    {
        throw std::runtime_error("Unable to initialize GLFW window!");
    }

    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        throw std::runtime_error("Unable to initialize GLAD!");
    }

    glEnable(GL_MULTISAMPLE);
}

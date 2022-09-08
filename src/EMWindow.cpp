#include "EMWindow.hpp"

#include <unordered_map>

#include "GLInclude.hpp"

static std::unordered_map<GLFWwindow*, EMWindow*> windowHandles;
static bool m_gladHasInit = false;

static void i_onWindowResize(GLFWwindow*, int, int);

EMWindow::EMWindow(int width, int height, const char* title) :
    m_width(width),
    m_height(height),
    m_numFrames(0)
{
    m_glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);

    glfwSetWindowSizeCallback(m_glfwWindow, i_onWindowResize);

    glfwMakeContextCurrent(m_glfwWindow);

    if(!m_gladHasInit)
    {
        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        m_gladHasInit = true;
    }

    windowHandles[m_glfwWindow] = this;
}

GLFWwindow* EMWindow::getHandle() const
{
    return m_glfwWindow;
}

int EMWindow::getWidth() const
{
    return m_width;
}

int EMWindow::getHeight() const
{
    return m_height;
}

unsigned int EMWindow::numFrames() const
{
    return m_numFrames;
}

bool EMWindow::shouldClose() const
{
    return glfwWindowShouldClose(m_glfwWindow);
}

void EMWindow::swapBuffers()
{
    glfwSwapBuffers(m_glfwWindow);
    m_numFrames++;
}

static void i_onWindowResize(GLFWwindow* window, int width, int height)
{
    EMWindow* emWindow = windowHandles[window];

    emWindow->m_width = width;
    emWindow->m_height = height;
}
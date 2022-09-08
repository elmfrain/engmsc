#ifndef EMWINDOW_HPP
#define EMWINDOW_HPP

#include <GLFW/glfw3.h>

class EMWindow
{
public:
    EMWindow(int width, int height, const char* title);
    EMWindow(const EMWindow&) = delete;

    GLFWwindow* getHandle() const;

    int getWidth() const;
    int getHeight() const;
    unsigned int numFrames() const;

    bool shouldClose() const;

    void swapBuffers();

    // GLFW resize callback
    friend void i_onWindowResize(GLFWwindow*, int, int);
private:
    GLFWwindow* m_glfwWindow;
    int m_width;
    int m_height;

    unsigned int m_numFrames;
};

#endif // EMWINDOW_HPP
#ifndef EMWINDOW_HPP
#define EMWINDOW_HPP

#include "GLFWInclude.hpp"
#include "Keyboard.hpp"

#include <memory>

class EMWindow
{
public:
    EMWindow(int width, int height, const char* title);
    EMWindow(const EMWindow&) = delete;

    GLFWwindow* getHandle() const;
    const EMKeyboard& getKeyboard() const;

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

    std::unique_ptr<EMKeyboard> m_keyboard;

    unsigned int m_numFrames;
};

#endif // EMWINDOW_HPP
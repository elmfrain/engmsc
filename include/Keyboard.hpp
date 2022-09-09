#ifndef EM_MOUSE_HPP
#define EM_MOUSE_HPP

#define INT_KEYFLAGS_SIZE 88

#include "GLFWInclude.hpp"

class EMKeyboard
{
public:
    bool keyJustPressed(int key) const;
    bool isKeyPressed(int key) const;
    bool keyJustReleased(int key) const;
    bool isKeyRepeating(int key) const;
private:
    EMKeyboard(GLFWwindow* window);

    GLFWwindow* m_window;

    uint32_t m_keyPressedStates[INT_KEYFLAGS_SIZE];
    uint32_t m_keyDownStates[INT_KEYFLAGS_SIZE];
    uint32_t m_keyReleasedStates[INT_KEYFLAGS_SIZE];
    uint32_t m_keyRepeatingStates[INT_KEYFLAGS_SIZE];

    void pollInputs();

    friend class EMWindow;
    // GLFW Key Event Callback
    friend void i_onKeyEvent(GLFWwindow*, int, int, int, int);
};

#endif // EM_MOUSE_HPP
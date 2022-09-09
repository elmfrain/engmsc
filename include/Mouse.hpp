#ifndef EMMOUSE_HPP
#define EMMOUSE_HPP

#include "GLFWInclude.hpp"
#include <glm/glm.hpp>

class EMMouse
{
public:
    bool isButtonPressed(int button) const;
    bool buttonJustPressed(int button) const;
    bool buttonJustReleased(int button) const;

    float cursorX() const;
    float cursorY() const;
    float cursorXDelta() const;
    float cursorYDelta() const;

    bool justScrolled() const;
    float scrollDeltaX() const;
    float scrollDeltaY() const;
private:
    EMMouse(GLFWwindow* window);

    GLFWwindow* m_window;

    uint32_t m_mousePressedStates;
    uint32_t m_mouseDownStates;
    uint32_t m_mouseReleasedStates;

    glm::vec2 m_prevCursorPos;
    glm::vec2 m_cursorPos;
    glm::vec2 m_scrollDelta;
    bool m_justScrolled;

    void pollInputs();

    friend class EMWindow;
    // GLFW mouse callbacks
    friend void i_onMouseEvent(GLFWwindow*, int, int, int);
    friend void i_onCursorMove(GLFWwindow*, double, double);
    friend void i_onScroll(GLFWwindow*, double, double);
};

#endif // EMMOUSE_HPP
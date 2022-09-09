#include "Mouse.hpp"

#include <unordered_map>

std::unordered_map<GLFWwindow*, EMMouse*> mouseHandles;

static inline void i_setNthBit(uint32_t* flagBuffer, int bit);
static inline void i_clearNthBit(uint32_t* flagBuffer, int bit);
static inline int i_getNthBit(const uint32_t* flagBuffer, int bit);
static void i_onMouseEvent(GLFWwindow*, int, int, int);
static void i_onCursorMove(GLFWwindow*, double, double);
static void i_onScroll(GLFWwindow*, double, double);

EMMouse::EMMouse(GLFWwindow* window) :
    m_window(window),
    m_mousePressedStates(0),
    m_mouseDownStates(0),
    m_mouseReleasedStates(0),
    m_prevCursorPos(0.0f, 0.0f),
    m_cursorPos(0.0f, 0.0f),
    m_scrollDelta(0.0f, 0.0f),
    m_justScrolled(false)
{
    glfwSetMouseButtonCallback(m_window, i_onMouseEvent);
    glfwSetCursorPosCallback(m_window, i_onCursorMove);
    glfwSetScrollCallback(m_window, i_onScroll);

    mouseHandles[m_window] = this;
}

bool EMMouse::isButtonPressed(int button) const
{
    return i_getNthBit(&m_mouseDownStates, button);
}

bool EMMouse::buttonJustPressed(int button) const
{
    return i_getNthBit(&m_mousePressedStates, button);
}

bool EMMouse::buttonJustReleased(int button) const
{
    return i_getNthBit(&m_mouseReleasedStates, button);
}

float EMMouse::cursorX() const
{
    return m_cursorPos.x;
}

float EMMouse::cursorY() const
{
    return m_cursorPos.y;
}

float EMMouse::cursorXDelta() const
{
    return m_cursorPos.x - m_prevCursorPos.x;
}

float EMMouse::cursorYDelta() const
{
    return m_cursorPos.y - m_prevCursorPos.y;
}

bool EMMouse::justScrolled() const
{
    return m_justScrolled;
}

float EMMouse::scrollDeltaX() const
{
    return m_scrollDelta.x;
}

float EMMouse::scrollDeltaY() const
{
    return m_scrollDelta.y;
}

void EMMouse::pollInputs()
{
    m_mousePressedStates = 0;
    m_mouseReleasedStates = 0;

    m_prevCursorPos = m_cursorPos;
    m_justScrolled = false;
}

static inline void i_setNthBit(uint32_t* flagBuffer, int bit)
{
    uint32_t* flagChunk = flagBuffer + (bit / sizeof(int32_t));
    int bitPos = bit % 32;

    uint32_t set = ((uint32_t) 1) << bitPos;
    *flagChunk |= set;
}

static inline void i_clearNthBit(uint32_t* flagBuffer, int bit)
{
    uint32_t* flagChunk = flagBuffer + (bit / sizeof(int32_t));
    int bitPos = bit % 32;

    uint32_t set = ~(((uint32_t) 1) << bitPos);
    *flagChunk &= set;
}

static inline int i_getNthBit(const uint32_t* flagBuffer, int bit)
{
    const uint32_t* flagChunk = flagBuffer + (bit / sizeof(int32_t));
    int bitPos = bit % 32;

    uint32_t value = ((uint32_t) 1) << bitPos;
    value &= *flagChunk;
    value = value >> bitPos;

    return value;
}

static void i_onMouseEvent(GLFWwindow* window, int button, int action, int mods)
{
    EMMouse* emMouse = mouseHandles[window];

    if(action == GLFW_PRESS)
    {
        i_setNthBit(&emMouse->m_mousePressedStates, button);
        i_setNthBit(&emMouse->m_mouseDownStates, button);
    }
    else if(action == GLFW_RELEASE)
    {
        i_clearNthBit(&emMouse->m_mouseDownStates, button);
        i_setNthBit(&emMouse->m_mouseReleasedStates, button);
    }
}

static void i_onCursorMove(GLFWwindow* window, double cursorX, double cursorY)
{
    EMMouse* emMouse = mouseHandles[window];

    emMouse->m_cursorPos.x = (float) cursorX;
    emMouse->m_cursorPos.y = (float) cursorY;
}

static void i_onScroll(GLFWwindow* window, double scrollX, double scrollY)
{
    EMMouse* emMouse = mouseHandles[window];

    emMouse->m_scrollDelta.x = (float) scrollX;
    emMouse->m_scrollDelta.y = (float) scrollY;
    emMouse->m_justScrolled = true;
}
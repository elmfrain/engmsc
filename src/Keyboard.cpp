#include "Keyboard.hpp"

#include <unordered_map>

static std::unordered_map<GLFWwindow*, EMKeyboard*> keyboardHandles;

static inline void i_setNthBit(uint32_t* flagBuffer, int bit);
static inline void i_clearNthBit(uint32_t* flagBuffer, int bit);
static inline int i_getNthBit(const uint32_t* flagBuffer, int bit);
static void i_onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

EMKeyboard::EMKeyboard(GLFWwindow* window) :
    m_window(window),
    m_keyPressedStates{0},
    m_keyDownStates{0},
    m_keyReleasedStates{0},
    m_keyRepeatingStates{0}
{
    glfwSetKeyCallback(m_window, i_onKeyEvent);

    keyboardHandles[m_window] = this;
}

bool EMKeyboard::keyJustPressed(int key) const
{
    return i_getNthBit(m_keyPressedStates, key);
}

bool EMKeyboard::isKeyPressed(int key) const
{
    return i_getNthBit(m_keyDownStates, key);
}

bool EMKeyboard::keyJustReleased(int key) const
{
    return i_getNthBit(m_keyReleasedStates, key);
}

bool EMKeyboard::isKeyRepeating(int key) const
{
    return i_getNthBit(m_keyRepeatingStates, key);
}

void EMKeyboard::pollInputs()
{
    memset(m_keyPressedStates, 0, INT_KEYFLAGS_SIZE * sizeof(uint32_t));
    memset(m_keyReleasedStates, 0, INT_KEYFLAGS_SIZE * sizeof(uint32_t));
    memset(m_keyRepeatingStates, 0, INT_KEYFLAGS_SIZE * sizeof(uint32_t));
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

static void i_onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    EMKeyboard* emKeyboard = keyboardHandles[window];

    if(action == GLFW_PRESS)
    {
        i_setNthBit(emKeyboard->m_keyPressedStates, key);
        i_setNthBit(emKeyboard->m_keyDownStates, key);
    }
    else if(action == GLFW_RELEASE)
    {
        i_clearNthBit(emKeyboard->m_keyDownStates, key);
        i_setNthBit(emKeyboard->m_keyReleasedStates, key);
    }
    else if(action == GLFW_REPEAT)
    {
        i_setNthBit(emKeyboard->m_keyRepeatingStates, key);
    }
}
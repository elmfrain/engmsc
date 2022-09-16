#ifndef UIRENDER_HPP
#define UIRENDER_HPP

#include <stdint.h>
#include "Window.hpp"

typedef int32_t ColorARGB8;
typedef int32_t Direction;
typedef int32_t Anchor;

#define getVec4Color(intcolor)\
{   ((intcolor >> 16) & 0xFF) / 255.0f,\
    ((intcolor >>  8) & 0xFF) / 255.0f,\
    (intcolor & 0xFF) / 255.0f,\
    ((intcolor >> 24) & 0xFF) / 255.0f }

namespace emui
{
    enum
    {
        // Direction
        TO_BOTTOM, TO_RIGHT, TO_LEFT, TO_TOP,

        // Top Anchor
           TOP_LEFT,    TOP_CENTER,    TOP_RIGHT,
        // Center Anchor
               LEFT,        CENTER,        RIGHT,
        // Bottom Anchor
        BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT
    };

    void init();
    void setWindow(EMWindow& window);
    EMWindow& getWindow();

    void genQuad(float left, float top, float right, float bottom, ColorARGB8 color, uint8_t texId = 0);
    void genQuad(float left, float top, float right, float bottom, const glm::vec4& color, uint8_t texId = 0);
    void genGradientQuad(float left, float top, float right, float bottom, ColorARGB8 startColor,
                         ColorARGB8 endColor, Direction dir = TO_BOTTOM, uint8_t texId = 0);
    void genGradientQuad(float left, float top, float right, float bottom, const glm::vec4& startColor,
                         const glm::vec4& endColor, Direction dir = TO_BOTTOM, uint8_t texId = 0);
    void genVerticalLine(float x, float startY, float endY, ColorARGB8 color, float thickness = 1.0f);
    void genVerticalLine(float x, float startY, float endY, const glm::vec4& color, float thickness = 1.0f);
    void genHorizontalLine(float y, float startX, float endX, ColorARGB8 color, float thickness = 1.0f);
    void genHorizontalLine(float y, float startX, float endX, const glm::vec4& color, float thickness = 1.0f);
    void genLine(float x1, float y1, float x2, float y2, ColorARGB8 color, float thickness = 1.0f);
    void genLine(float x1, float y1, float x2, float y2, const glm::vec4& color, float thickness = 1.0f);
    void genString(const char* text, float x, float y, ColorARGB8 color, Anchor anchor);

    float getUIWidth();
    float getUIHeight();
    float getUIScale();

    void pushStack();
    void scale(float x, float y);
    void translate(float x, float y);
    void rotate(float x, float y, float z);
    void mult(const glm::mat4& val);
    void popStack();
    glm::mat4& getModelView();

    void setupUIRendering();
    void renderBatch();
}

#endif // UIRENDER_HPP
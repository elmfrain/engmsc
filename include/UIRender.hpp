#ifndef UIRENDER_HPP
#define UIRENDER_HPP

#include <stdint.h>
#include "EMWindow.hpp"

typedef int32_t ColorARGB8;
typedef int32_t Direction;
typedef int32_t Anchor;

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

    void drawRect(float left, float top, float right, float bottom, ColorARGB8 color);
    void drawGradientRect(float left, float top, float right, float bottom, Direction dir, ColorARGB8 startColor, ColorARGB8 endColor);
    void drawVerticalLine(float x, float startY, float endY, ColorARGB8 color);
    void drawHorizontalLine(float y, float startX, float endX, ColorARGB8 color);
    void drawLine(float x1, float y1, float x2, float y2, ColorARGB8 color);
    void drawString(const char* text, float x, float y, Anchor anchor, ColorARGB8);

    void renderBatch();

    void init();
    void setWindow(const EMWindow& window);
}

#endif // UIRENDER_HPP
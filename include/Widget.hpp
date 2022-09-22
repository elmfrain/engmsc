#ifndef EMWIDGET_HPP
#define EMWIDGET_HPP

#include <glm/glm.hpp>
#include <string>
#include <stack>

class EMWidget
{
public:
    enum Type
    {
        WIDGET,
        BUTTON,
        VIEWPORT,
        GAUGE,
    };

    static int getCurrentZLevel();
    static void setCurrentZLevel(int zLevel);

    EMWidget();
    EMWidget(const EMWidget& copy) = delete;
    virtual ~EMWidget();

    bool isHovered() const;
    bool isVisible() const;
    bool isEnabled() const;
    bool isOnCurrentZLevel() const;
    bool isPressed() const;
    bool justPressed() const;
    bool justReleased() const;

    void setEnabled(bool enabled);
    void setVisible(bool visible);
    void setZLevel(int zLevel);
    void setText(const char* text);

    Type getType() const;
    int getZLevel() const;
    const char* getText() const;

    float localCursorX() const;
    float localCursorY() const;

    void draw();

    float x, y, width, height;
protected:
    // Clipping (with viewports)
    struct ClippingState
    {
        glm::mat4 modelView;
        float left, right, top, bottom;
        bool clipping = false;
    };
    static std::stack<ClippingState> m_clippingStack;

    glm::mat4 m_modelView;

    // Other Types of Widgets Overrides this
    virtual void doDraw();
    Type m_type;
private:
    bool m_hovered;
    bool m_visible;
    bool m_enabled;
    int m_zLevel;

    glm::vec2 m_localCursor;

    bool m_justPressed;
    bool m_pressed;
    bool m_justReleased;
    std::string m_text;

    void updateHoverState();
};

#endif // EMWIDGET_HPP
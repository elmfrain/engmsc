#include "Widget.hpp"

#include "UIRender.hpp"
#include "Shaders.hpp"

// Clipping (with viewports)
std::stack<EMWidget::ClippingState> EMWidget::m_clippingStack;

static int m_currentZLevel = 0;

int EMWidget::getCurrentZLevel()
{
    return m_currentZLevel;
}

void EMWidget::setCurrentZLevel(int zLevel)
{
    m_currentZLevel = zLevel;
}

EMWidget::EMWidget() :
    x(0.0f), y(0.0f), width(0.0f), height(0.0f),
    m_hovered(false),
    m_visible(true),
    m_enabled(true),
    m_zLevel(0),
    m_type(WIDGET),
    m_modelView(1.0f),
    m_localCursor(0.0f, 0.0f),
    m_justPressed(false),
    m_pressed(false),
    m_justReleased(false)
{
}

EMWidget::~EMWidget()
{

}

bool EMWidget::isHovered() const
{
    return m_hovered;
}

bool EMWidget::isVisible() const
{
    return m_visible;
}

bool EMWidget::isEnabled() const
{
    return m_enabled;
}

bool EMWidget::isOnCurrentZLevel() const
{
    return m_zLevel == m_currentZLevel;
}

bool EMWidget::isPressed() const
{
    return m_pressed;
}

bool EMWidget::justPressed() const
{
    return m_justPressed;
}

bool EMWidget::justReleased() const
{
    return m_justReleased;
}

void EMWidget::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void EMWidget::setVisible(bool visible)
{
    m_visible = visible;
}

void EMWidget::setZLevel(int zLevel)
{
    m_zLevel = zLevel;
}

void EMWidget::setText(const char* text)
{
    m_text = text;
}

EMWidget::Type EMWidget::getType() const
{
    return m_type;
}

int EMWidget::getZLevel() const
{
    return m_zLevel;
}

const char* EMWidget::getText() const
{
    return m_text.c_str();
}

float EMWidget::localCursorX() const
{
    return m_localCursor.x;
}

float EMWidget::localCursorY() const
{
    return m_localCursor.y;
}

void EMWidget::draw()
{
    m_modelView = ems::getModelviewMatrix();
    m_modelView = m_modelView * emui::getModelView();

    const EMMouse& mouse = emui::getWindow().getMouse();
    updateHoverState();

    doDraw();

    m_justPressed = m_justReleased = false;
    if(mouse.buttonJustPressed(GLFW_MOUSE_BUTTON_1) && m_hovered)
    {
        m_justPressed = true;
        m_pressed = true;
    }
    else if(mouse.buttonJustReleased(GLFW_MOUSE_BUTTON_1))
    {
        m_pressed = false;
        m_justReleased = true;
    }
}

void EMWidget::doDraw()
{
    emui::genQuad(x, y, x + width, y + height, 0xFFFFFFFF);
}

void EMWidget::updateHoverState()
{
    const EMMouse& mouse = emui::getWindow().getMouse();

    if(m_clippingStack.empty())
    {
        m_clippingStack.emplace();
    }

    ClippingState clip = m_clippingStack.top();

    glm::mat4 inverseModelView = glm::inverse(clip.modelView);
    glm::vec4 uiCursorWidget(
        mouse.cursorX() / emui::getUIScale(),
        mouse.cursorY() / emui::getUIScale(),
        0.0f,
        1.0f
    );
    glm::vec4 uiCursorClip = uiCursorWidget;

    uiCursorClip = inverseModelView * uiCursorClip;

    bool cursorInBounds = clip.clipping ? uiCursorClip.x > clip.left && uiCursorClip.y > clip.top && uiCursorClip.x < clip.right && uiCursorClip.y < clip.bottom : true;
    
    inverseModelView = glm::inverse(m_modelView);

    uiCursorWidget = inverseModelView * uiCursorWidget;
    m_localCursor.x = uiCursorWidget.x - x;
    m_localCursor.y = uiCursorWidget.y - y;

    bool hovered = uiCursorWidget.x >= x && uiCursorWidget.y >= y && uiCursorWidget.x < x + width && uiCursorWidget.y < y + height;

    m_hovered = isOnCurrentZLevel() && hovered && cursorInBounds;
}
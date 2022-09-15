#include "Button.hpp"

#include "UIRender.hpp"

EMButton::EMButton()
{
    width = 180;
    height = 30;
}

EMButton::EMButton(const char* text)
{
    setText(text);

    width = 180;
    height = 30;
}

void EMButton::doDraw()
{
    int textColor = !isEnabled() ? 0xFFA0A0A0 : isHovered() ? 0xFFFFFFBA :0xFFE0E0E0;
    int backgroundColor = isHovered() ? 0x664C4C4C : 0x66000000;
    emui::genQuad(x, y, x + width, y + height, backgroundColor);
    emui::genString(getText(), x + width / 2, y + height / 2, textColor, emui::CENTER);
}
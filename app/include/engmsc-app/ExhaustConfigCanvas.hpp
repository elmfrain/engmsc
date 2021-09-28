#ifndef EXHAUST_CONFIG_CANVAS
#define EXHAUST_CONFIG_CANVAS

#include <nanogui/canvas.h>

class ExhaustConfigCanvas : public nanogui::Canvas
{
public:
    ExhaustConfigCanvas(nanogui::Widget* parent);

    virtual void draw_contents() override;
};

#endif
#ifndef EXHAUST_CONFIG_CANVAS
#define EXHAUST_CONFIG_CANVAS

#include <nanogui/canvas.h>
#include <functional>

class ExhaustConfigCanvas : public nanogui::Canvas
{
public:
    ExhaustConfigCanvas(nanogui::Widget* parent);

    virtual void draw_contents() override;
    virtual bool mouse_button_event(const nanogui::Vector2i& pos, int button, bool down, int modifiers) override;
    void setNbCylinders(int cyl);
    void setOffsets(const float* offsets, int length);
    void setCallback(std::function<void()>);
    int getSelectedCylinder() const;
private:
    std::function<void()> m_callback = nullptr;
    int m_nbCylinders = 1;
    int m_selectedCylinder = 1;
    float m_offsets[16] = { 0.0f };
};

#endif
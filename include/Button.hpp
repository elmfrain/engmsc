#ifndef EMBUTTON_HPP
#define EMBUTTON_HPP

#include "Widget.hpp"

class EMButton : public EMWidget
{
public:
    EMButton();
    EMButton(const char* text);
protected:
    virtual void doDraw();
private:
};

#endif // EMBUTTON_HPP
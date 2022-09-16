#ifndef EMVIEWPORT_HPP
#define EMVIEWPORT_HPP

#include "Widget.hpp"

class EMViewport : public EMWidget
{
public:
    EMViewport();

    void start(bool clipping);
    void end();
protected:
    virtual void doDraw();
private:
    bool m_wasClipping;
};

#endif // EMVIEWPORT_HPP
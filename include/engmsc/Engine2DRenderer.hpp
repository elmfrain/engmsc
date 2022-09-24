#ifndef EMENGINE2DRENDERER_HPP
#define EMENGINE2DRENDERER_HPP

struct EMEngine
{
    double crankAngle;
    double crankSpeed;
};

namespace EMEngine2DRenderer
{
    void init();

    EMEngine& getEngine();

    void render();
}

#endif // EMENGINE2DRENDERER_HPP
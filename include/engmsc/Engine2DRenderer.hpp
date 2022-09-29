#ifndef EMENGINE2DRENDERER_HPP
#define EMENGINE2DRENDERER_HPP

struct EMEngine
{
    struct Profile
    {
        Profile();

        float stroke;
        float bore;
        float rodLength;

        float camIntakeAngle;
        float camIntakeLift;
        float camIntakeDuration;
        float camExhaustAngle;
        float camExhaustLift;
        float camExhaustDuration;
    };

    Profile profile;
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
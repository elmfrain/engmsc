#ifndef EMENGINE_HPP
#define EMENGINE_HPP

#include <vector>

// Contains parameters for a single cylinder (cannot be used on its own)
struct EMEngineCylinder
{
    EMEngineCylinder();

    float stroke;
    float bore;
    float rodLength;
    float deckClearance;
    float gasketHeight;
    float combustionChamberVolume;

    float intakeValveRadius;
    float intakePortArea;
    float intakeRunnerVolume;
    float camIntakeAngle;
    float camIntakeLift;
    float camIntakeDuration;

    float exhaustValveRadius;
    float exhaustPortArea;
    float exhaustRunnerVolume;
    float camExhaustAngle;
    float camExhaustLift;
    float camExhaustDuration;

    double blowbyArea;
    double pistonFrictionForce;
    double angleOffset;
    double bankAngle;

    bool headFlipped;
};

// Complete engine with one or more cylinders (ready for physics simulations)
struct EMEngineAssembly
{
    EMEngineAssembly();

    std::vector<EMEngineCylinder> cylinders;
    double crankAngle;
    double crankSpeed;

    double rotationalMass;
    double frictionTorque;

    double trottle;
};

#endif // EMENGINE_HPP
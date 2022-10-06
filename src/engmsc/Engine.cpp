#include "engmsc/Engine.hpp"

// Default profile
EMEngineCylinder::EMEngineCylinder() :
    stroke(1.0f),
    bore(1.0f),
    rodLength(1.2f),
    deckClearance(0.04f),
    gasketHeight(0.0024f),
    combustionChamberVolume(7.38e-3f),
    intakeValveRadius(0.175f),
    camIntakeAngle(-1.004f),
    camIntakeLift(0.068f),
    camIntakeDuration(2.217f),
    exhaustValveRadius(0.15f),
    camExhaustAngle(1.004f),
    camExhaustLift(0.068f),
    camExhaustDuration(2.217f),
    blowbyArea(3.596e-6),
    pistonFrictionForce(5.0),
    angleOffset(0.0),
    bankAngle(0.0),
    headFlipped(false)
{
}

EMEngineAssembly::EMEngineAssembly() :
    crankAngle(0.0),
    crankSpeed(0.0),
    rotationalMass(1.0),
    frictionTorque(20.0)
{
    cylinders.emplace_back();
}
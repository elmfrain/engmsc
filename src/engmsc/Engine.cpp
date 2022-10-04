#include "engmsc/Engine.hpp"

// Default profile
EMEngineCylinder::EMEngineCylinder() :
    stroke(1.0f),
    bore(1.0f),
    rodLength(1.2f),
    camIntakeAngle(-1.004f),
    camIntakeLift(0.068f),
    camIntakeDuration(2.217f),
    camExhaustAngle(1.004f),
    camExhaustLift(0.068f),
    camExhaustDuration(2.217f),
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
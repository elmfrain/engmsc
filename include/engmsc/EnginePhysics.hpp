#ifndef EMENGINE_PHYSICS_HPP
#define EMENGINE_PHYSICS_HPP

#include "engmsc/Engine.hpp"

namespace EMEnginePhysics
{
    void setEngineAssembly(EMEngineAssembly& engine);

    void applyTorque(double torque);

    void update(double timeDelta, int steps);

    double getCylPressure(int cylNum);
}

#endif // EMENGINE_PHYSICS_HPP
#ifndef EMENGINE_PHYSICS_HPP
#define EMENGINE_PHYSICS_HPP

#include "engmsc/Engine.hpp"

namespace EMEnginePhysics
{
    void setEngineAssembly(EMEngineAssembly& engine);
    EMEngineAssembly& getEngineAssembly();

    void applyTorque(double torque);

    void update(double timeDelta, int steps);

    double getCylPressure(int cylNum);
    double getIntakePressure(int cylNum);
    double getExhaustPressure(int cylNum);

    double getCylGasMass(int cylNum);
    double getExhaustGasMass(int cylNum);

    double getIntakeAirVelocity(int cylNum);
}

#endif // EMENGINE_PHYSICS_HPP
#include "engmsc/EnginePhysics.hpp"

#include "Logger.hpp"

#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define HALF_PI glm::half_pi<double>()
#define PI glm::pi<double>()
#define THREE_HALF_PI glm::three_over_two_pi<double>()
#define TWO_PI glm::two_pi<double>()

static EMLogger m_logger("Engine Physics");
static bool m_advised = false;

static double m_simulationTime = 0.0;
static EMEngineAssembly* m_engine = NULL;

static double m_externalTorque = 0.0;

static void i_simulationStep(double timeDelta);

void EMEnginePhysics::setEngineAssembly(EMEngineAssembly& engine)
{
    m_engine = & engine;
}

void EMEnginePhysics::applyTorque(double torque)
{
    m_externalTorque = torque;
}

void EMEnginePhysics::update(double timeDelta, int steps)
{
    if(!m_engine)
    {
        if(!m_advised)
        {
            m_logger.errorf("Cannot do simulation, no engine is set!");
            m_advised = true;
        }
        return;
    }
    m_advised = false;

    assert(0 < steps);

    double stepTimeInterval = timeDelta / steps;

    for(int i = 0; i < steps; i++)
    {
        i_simulationStep(stepTimeInterval);
    }
    m_simulationTime += timeDelta;
    m_externalTorque = 0.0;
}

static void i_simulationStep(double timeDelta)
{
    double totalTorque = m_externalTorque;

    for(EMEngineCylinder& cyl : m_engine->cylinders)
    {
        double cylCrankAngle = m_engine->crankAngle + cyl.angleOffset;

        double b = -cylCrankAngle + HALF_PI;
        double pistonY =
        glm::sin(glm::acos(glm::cos(b) * cyl.stroke / (2.0 * cyl.rodLength))) * cyl.rodLength
        + glm::sin(b) * cyl.stroke / 2.0;
        double conrodAngle = glm::asin(glm::sin(cylCrankAngle) / cyl.rodLength);

        double pistonForce = 0.0;
        totalTorque += pistonForce * glm::sin(conrodAngle) * pistonY;
    }

    double frictionTorque = 0.0 < m_engine->crankSpeed ? m_engine->frictionTorque : -m_engine->frictionTorque;

    if(-0.01 < m_engine->crankSpeed && m_engine->crankSpeed < 0.01)
    {
        m_engine->crankSpeed = 0.0;
        frictionTorque = 0.0;
    }

    double acceleration = (totalTorque - frictionTorque) / m_engine->rotationalMass;
    m_engine->crankSpeed += acceleration * timeDelta;
    m_engine->crankAngle += m_engine->crankSpeed * timeDelta;
}
#include "engmsc/EnginePhysics.hpp"

#include "Logger.hpp"

#include <vector>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define HALF_PI glm::half_pi<double>()
#define PI glm::pi<double>()
#define THREE_HALF_PI glm::three_over_two_pi<double>()
#define TWO_PI glm::two_pi<double>()

#define ONE_ATM_PRES 101325
#define AIR_VISCOSITY 1.798e-5

struct CylinderDynamics
{
    // Pre-calculated variables
    double tdcPistonY = 0.0;
    double bdcPistonY = 0.0;
    double minCylVolume = 0.0;
    double pistonArea = 0.0;
    double blowbyAmount = 0.0;

    // Dynamic varaibles
    double cylVolume = 0.0;
    double cylPressure = ONE_ATM_PRES;
};

static EMLogger m_logger("Engine Physics");

static bool m_advised = false;
static double m_simulationTime = 0.0;

// Engine Reference
static EMEngineAssembly* m_engine = NULL;
static std::vector<CylinderDynamics> m_cylDynamics;

static double m_externalTorque = 0.0;

static inline double i_getPistonY(EMEngineCylinder& cyl, double angle);
static void i_simulationStep(double timeDelta);

void EMEnginePhysics::setEngineAssembly(EMEngineAssembly& engine)
{
    m_cylDynamics.clear();

    m_engine = &engine;

    // Setup and make pre-calculations per cylinder
    for(EMEngineCylinder& cyl : m_engine->cylinders)
    {
        CylinderDynamics& cylDym = m_cylDynamics.emplace_back();

        double pistonArea = PI * glm::pow(cyl.bore / 2.0, 2.0);

        cylDym.tdcPistonY = cyl.stroke / 2.0 + cyl.rodLength;
        cylDym.bdcPistonY = -cyl.stroke / 2.0 + cyl.rodLength;
        cylDym.minCylVolume = 
        cyl.combustionChamberVolume + cyl.gasketHeight * pistonArea + cyl.deckClearance * pistonArea;
        cylDym.pistonArea = pistonArea;
        cylDym.blowbyAmount = cyl.blowbyArea / AIR_VISCOSITY;

        cylDym.cylVolume =
        (cylDym.tdcPistonY - i_getPistonY(cyl, cyl.angleOffset)) * cylDym.pistonArea + cylDym.minCylVolume;
    }
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

double EMEnginePhysics::getCylPressure(int cylNum)
{
    assert(cylNum < m_cylDynamics.size());

    return m_cylDynamics[cylNum].cylPressure;
}

static inline double i_getPistonY(EMEngineCylinder& cyl, double angle)
{
    angle = -angle + HALF_PI;
     return glm::sin(glm::acos(glm::cos(angle) * cyl.stroke / (2.0 * cyl.rodLength))) * cyl.rodLength
        + glm::sin(angle) * cyl.stroke / 2.0;
}

static void i_simulationStep(double timeDelta)
{
    double netTorque = m_externalTorque;
    double netFirctionTorque = m_engine->frictionTorque;

    // Per cylinder dynamics simulation
    int numCylinders = (int) m_engine->cylinders.size();
    for(int i = 0; i < numCylinders; i++)
    {
        EMEngineCylinder& cyl = m_engine->cylinders[i];
        CylinderDynamics& cylDym = m_cylDynamics[i];
        double cylCrankAngle = m_engine->crankAngle + cyl.angleOffset;

        double pistonY = i_getPistonY(cyl, cylCrankAngle);
        double conrodAngle = glm::asin(glm::sin(cylCrankAngle) / cyl.rodLength);
        double torqueDist = glm::sin(conrodAngle) * pistonY;

        double newVolume = (cylDym.tdcPistonY - pistonY) * cylDym.pistonArea + cylDym.minCylVolume;
        cylDym.cylPressure = cylDym.cylPressure * cylDym.cylVolume / newVolume;

        cylDym.cylPressure += (ONE_ATM_PRES - cylDym.cylPressure) * cylDym.blowbyAmount * timeDelta;

        double atmForce = ONE_ATM_PRES * cylDym.pistonArea;
        double pistonForce = cylDym.pistonArea * cylDym.cylPressure - atmForce;

        cylDym.cylVolume = newVolume;
        netTorque += pistonForce * torqueDist;
        netFirctionTorque += cyl.pistonFrictionForce * glm::abs(torqueDist);
    }

    double frictionTorque = 0.0 < m_engine->crankSpeed ? netFirctionTorque : -netFirctionTorque;
    double frictionSpeed = (netFirctionTorque / m_engine->rotationalMass) * timeDelta;

    if(-frictionSpeed < m_engine->crankSpeed && m_engine->crankSpeed < frictionSpeed)
    {
        m_engine->crankSpeed = 0.0;
        frictionTorque = 0.0;
        if(-netFirctionTorque < netTorque && netTorque < netFirctionTorque)
        {
            frictionTorque = netTorque;
        }
    }

    double acceleration = (netTorque - frictionTorque) / m_engine->rotationalMass;
    m_engine->crankSpeed += acceleration * timeDelta;
    m_engine->crankAngle += m_engine->crankSpeed * timeDelta;
}
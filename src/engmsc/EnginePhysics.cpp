#include "engmsc/EnginePhysics.hpp"

#include "Logger.hpp"
#include "engmsc/EngineAudio.hpp"

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
    double intakeValveArea = 0.0;
    double exhaustValveArea = 0.0;

    // Dynamic varaibles
    double cylVolume = 0.0;
    double cylPressure = ONE_ATM_PRES;
    double intakePressure = ONE_ATM_PRES;
    double exhaustPressure = ONE_ATM_PRES;
};

static EMLogger m_logger("Engine Physics");

static bool m_advised = false;
static double m_simulationTime = 0.0;

// Engine Reference
static EMEngineAssembly* m_engine = NULL;
static std::vector<CylinderDynamics> m_cylDynamics;

static double m_externalTorque = 0.0;

static inline double i_getPistonY(EMEngineCylinder& cyl, double angle);
static inline double i_getCamLift(double angle, float duration, float lift, float offsetAngle);
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
        cylDym.intakeValveArea = PI * glm::pow(cyl.intakeValveRadius, 2.0);
        cylDym.exhaustValveArea = PI * glm::pow(cyl.exhaustValveRadius, 2.0);

        cylDym.cylVolume =
        (cylDym.tdcPistonY - i_getPistonY(cyl, cyl.angleOffset)) * cylDym.pistonArea + cylDym.minCylVolume;
    }
}

EMEngineAssembly& EMEnginePhysics::getEngineAssembly()
{
    assert(m_engine != NULL);
    return *m_engine;
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
    assert(cylNum < m_cylDynamics.size() && -1 < cylNum);

    return m_cylDynamics[cylNum].cylPressure;
}

double EMEnginePhysics::getExhaustPressure(int cylNum)
{
    assert(cylNum < m_cylDynamics.size() && -1 < cylNum);

    return m_cylDynamics[cylNum].exhaustPressure;
}

static inline double i_getPistonY(EMEngineCylinder& cyl, double angle)
{
    angle = -angle + HALF_PI;
     return glm::sin(glm::acos(glm::cos(angle) * cyl.stroke / (2.0 * cyl.rodLength))) * cyl.rodLength
        + glm::sin(angle) * cyl.stroke / 2.0;
}

static inline double i_getCamLift(double angle, float duration, float lift, float offsetAngle)
{
    angle = glm::mod(angle / 2.0 + offsetAngle - PI, TWO_PI);
    double x = 0.0;
    if(PI - 0.5 * duration < angle && angle < PI + 0.5 * duration)
    { x = glm::cos(TWO_PI * (angle - PI) / duration) * 0.5 * lift + 0.5 * lift; }
    return x;
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

        // Calculate piston location
        double pistonY = i_getPistonY(cyl, cylCrankAngle);
        double conrodAngle = glm::asin(glm::sin(cylCrankAngle) / cyl.rodLength);
        double torqueDist = glm::sin(conrodAngle) * pistonY;

        // Calculate cylinder volume and pressure from the result
        double newVolume = (cylDym.tdcPistonY - pistonY) * cylDym.pistonArea + cylDym.minCylVolume;
        cylDym.cylPressure = cylDym.cylPressure * cylDym.cylVolume / newVolume;

        // Calculate cylinder pressure from blowby
        cylDym.cylPressure += (ONE_ATM_PRES - cylDym.cylPressure) * cylDym.blowbyAmount * timeDelta;

        // Calculate cylinder pressure depending on the intake valve's position
        double intakeAir = cylDym.intakeValveArea * 
        i_getCamLift(cylCrankAngle, cyl.camIntakeDuration, cyl.camIntakeLift, cyl.camIntakeAngle) /
        AIR_VISCOSITY;
        cylDym.cylPressure += (cylDym.intakePressure - cylDym.cylPressure) * intakeAir * timeDelta;

        // Calculate intake pressure
        cylDym.intakePressure += (cylDym.cylPressure - cylDym.intakePressure) * intakeAir * timeDelta;
        cylDym.intakePressure +=
        (ONE_ATM_PRES - cylDym.intakePressure) * (cyl.intakePortArea / AIR_VISCOSITY) * timeDelta;

        // Calculate cylinder pressure depending on the exhaust valve's position
        double exhaustAir = cylDym.exhaustValveArea *
        i_getCamLift(cylCrankAngle, cyl.camExhaustDuration, cyl.camExhaustLift, cyl.camExhaustAngle) /
        AIR_VISCOSITY;
        cylDym.cylPressure += (cylDym.exhaustPressure - cylDym.cylPressure) * exhaustAir * timeDelta;

        // Calculate exhaust pressure
        cylDym.exhaustPressure += (cylDym.cylPressure - cylDym.exhaustPressure) * exhaustAir * timeDelta;
        cylDym.exhaustPressure += 
        (ONE_ATM_PRES - cylDym.exhaustPressure) * (cyl.exhaustPortArea / AIR_VISCOSITY) * timeDelta;

        // Calculate torque applied to the crank
        double atmForce = ONE_ATM_PRES * cylDym.pistonArea;
        double pistonForce = cylDym.pistonArea * cylDym.cylPressure - atmForce;
        cylDym.cylVolume = newVolume;
        netTorque += pistonForce * torqueDist;
        netFirctionTorque += cyl.pistonFrictionForce * glm::abs(torqueDist);

        // Send cylinder status to the audio renderer
        EMEngineAudio::CylinderStatus status;
        status.timeDelta = timeDelta;
        status.exhaustPressure = (float) cylDym.exhaustPressure;
        status.exhaustVelocity = (float) ((cylDym.exhaustPressure - cylDym.cylPressure) * exhaustAir);
        EMEngineAudio::m_engineLog.push_back(status);
    }

    // Calculate and handle friction
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

    // Calucate angle and speed of crank from torque applied
    double acceleration = (netTorque - frictionTorque) / m_engine->rotationalMass;
    m_engine->crankSpeed += acceleration * timeDelta;
    m_engine->crankAngle += m_engine->crankSpeed * timeDelta;
}
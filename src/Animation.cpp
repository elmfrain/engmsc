#include "Animation.hpp"

#include "GLFWInclude.hpp"

#include <glm/glm.hpp>

EMSmoother::EMSmoother() :
    m_acceleration(0.0),
    m_velocity(0.0),
    m_previousTime(0.0),
    m_currentTime(0.0),
    m_grabbed(false),
    m_grabbingTo(0.0),
    m_value(0.0),
    m_springy(false),
    m_speed(10.0),
    m_friction(1.0)
{
}

void EMSmoother::grab()
{
    m_grabbed = true;
}

void EMSmoother::grab(double grabTo)
{
    m_grabbed = true;
    m_grabbingTo = grabTo;
}

void EMSmoother::setValueAndGrab(double value)
{
    m_grabbed = true;
    m_grabbingTo = value;
    m_value = value;
}

void EMSmoother::release()
{
    m_grabbed = false;
}

bool EMSmoother::isGrabbed() const
{
    return m_grabbed;
}

bool EMSmoother::isSpringy() const
{
    return m_springy;
}

void EMSmoother::setSpringy(bool springy)
{
    m_springy = springy;
}

void EMSmoother::setSpeed(double speed)
{
    m_speed = speed;
}

void EMSmoother::setFriction(double friciton)
{
    m_friction = friciton;
}

void EMSmoother::setValue(double value)
{
    m_value = value;
}

double EMSmoother::getSpeed() const
{
    return m_speed;
}

double EMSmoother::getFriction() const
{
    return m_friction;
}

double EMSmoother::grabbingTo() const
{
    return m_grabbingTo;
}

double EMSmoother::getValue()
{
    update();
    return m_value;
}

float EMSmoother::getValuef()
{
    update();
    return (float) m_value;
}

void EMSmoother::update()
{
    m_previousTime = m_currentTime;
    m_currentTime = glfwGetTime();
    
    double delta = m_currentTime - m_previousTime;

    if(0.2 < delta) delta = 0.2;

    if(m_grabbed)
    {
        if(m_acceleration)
        {
            m_acceleration = (m_grabbingTo - m_value) * 32.0;
            m_velocity += m_acceleration * delta;
            m_velocity *= glm::pow(0.0025 / m_speed, delta);
        }
        else m_velocity = (m_grabbingTo - m_value) * m_speed;
    }

    m_value += m_velocity * delta;
    m_velocity *= glm::pow(0.0625 / (m_speed * m_friction), delta);
}

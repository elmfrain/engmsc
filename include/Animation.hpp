#ifndef EMANIMATION_HPP
#define EMANIMATION_HPP

class EMSmoother
{
public:
    EMSmoother();

    void grab();
    void grab(double grabTo);
    void setValueAndGrab(double value);

    void release();

    bool isGrabbed() const;
    bool isSpringy() const;

    void setSpringy(bool springy);
    void setSpeed(double speed);
    void setFriction(double friciton);
    void setValue(double value);

    double getSpeed() const;
    double getFriction() const;
    double grabbingTo() const;
    double getValue();
    float getValuef();
private:
    double m_acceleration;
    double m_velocity;
    double m_previousTime;
    double m_currentTime;

    bool m_grabbed;
    double m_grabbingTo;
    double m_value;

    bool m_springy;
    double m_speed;
    double m_friction;

    void update();
};

class EMTimer
{

public:
    EMTimer(double tps);

    double tps;
private:
    double m_nextTick;
};

#endif // EMANIMAITON_HPP
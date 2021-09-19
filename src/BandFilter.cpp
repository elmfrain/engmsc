#include <engmsc/BandFilter.hpp>

#include <math.h>

void BandFilter::setSpeed(double speed)
{
    this->speed = speed;
}

double BandFilter::getSpeed() const
{
    return speed;
}

double BandFilter::getValue()
{
    return value;
}

void BandFilter::sampleIn(double sample, double delta)
{
    velocity = (sample - value) * abs(speed);

    value += velocity * delta;
    velocity *= pow(0.0625 / speed, delta);
}
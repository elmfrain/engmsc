#pragma once 

#ifndef BAND_FILTER_HPP
#define BAND_FILTER_HPP

class BandFilter
{
private:
    double velocity = 0.0;
    double previousTime = 0;
    double currentTime = 0;
    
    double grabbingTo = 0.0;

    double value = 0.0;
    double speed = 1.0;
    
public:
    void setSpeed(double speed);
    double getSpeed() const;
    double getValue();
    void sampleIn(double value, double timeDelta);
};

#endif
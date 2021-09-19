#pragma once

class FlywheelRenderer
{
public:
    struct Engine
    {
        double rpm = 0.0;
        double coolantTemperature = 40;
        double throttle = 0.0;
        double brake = 0.0;
        double revLimiter = 7500.0;
        double idle = 750.0;
        double idleTolerance = 300.0;
        double angle = 0.0;
        double angleSpeed = 0.0;
        bool limiterOn = false;
    };

    static void initialize();
    static void draw();

    static Engine* getEngine();
};
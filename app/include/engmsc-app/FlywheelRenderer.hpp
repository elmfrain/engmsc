#pragma once

class FlywheelRenderer
{
public:
    struct Engine
    {
        double rpm = 0.0;
        double airFuelMassIntake = 0.0;
        double coolantTemperature = 21.0;
        double throttle = 0.0;
        double brake = 0.0;
        double revLimit = 7500.0;
        double idleTarget = 700.0;
        double idleThrottle = 0.0;
        double angle = 0.0;
        double angleSpeed = 0.0;
        bool limiterOn = false;
        bool isCranking = false;
    };

    static void initialize();
    static void draw();

    static Engine* getEngine();
};
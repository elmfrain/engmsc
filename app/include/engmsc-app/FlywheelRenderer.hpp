#pragma once

class FlywheelRenderer
{
public:
    struct Engine
    {
        double rpm = 0.0;
        double torque = 0.0; //in N*m
        double airFuelMassIntake = 0.0; //In grams/second
        double coolantTemperature = 21.0; //In celcius
        double throttle = 0.0;
        double brake = 0.0;
        double revLimit = 7500.0;
        double idleTarget = 700.0;//In revs/min
        double idleThrottle = 0.0;
        double angle = 0.0; //In degrees
        double angleSpeed = 0.0; //In degrees/frame
        double rotationalMass = 0.65; //In kg*m^2 
        bool limiterOn = false;
        bool isCranking = false;
    };

    struct Gearbox
    {
        double brakeAmount = 0.0;
        double clutchAmount = 0.0;
        double wheelAngleSpeed = 0.0; //In degrees/frame
        double kmh = 0.0;
        double vehicleAeroDrag = 0.15;
        double vehicleMass = 1315.0; //In kg
        int gear = 0;
        float gearRatios[7] = { 5.031f, 3.15f, 2.04f, 1.54f, 1.22f, 1.0f, 0.87f };
        float finalDriveRatio = 2.74f;
        float tireRadius = 0.2667f; //In meters

        void setGear(int gear);
    };
    

    static void initialize();
    static void draw();

    static Engine* getEngine();
    static Gearbox* getGearbox();
};
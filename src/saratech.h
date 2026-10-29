#ifndef SARATECH_H
#define SARATECH_H

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

// Main SARATECH library header
// Includes all components and provides global instances

#include "RobotState.h"
#include "MotorDriver.h"
#include "GTD35Adapter.h"
#include "TDAxis12Adapter.h"
#include "ServoAdapter.h"
#include "Sina.h"
#include "Sadra.h"
#include "Amirreza.h"
#include "King.h"

// Global instances (defined in saratech.cpp)
extern Sina sina;
extern Sadra sadra;
extern Amirreza amirreza;
extern King king;
extern MotorDriver motorDriver;
extern GTD35Adapter gtd35Adapter;
extern TDAxis12Adapter tdaAxis12;
extern ServoAdapter servoAdapter;

// Library initialization
void saratechBegin(TwoWire* i2cWire = &Wire, uint8_t tdaAddress = 0x10);

// Sensor update function (call once per loop)
void saratechUpdateSensors();

// Helper functions
namespace saratech {
    // Angle normalization
    inline float normalizeAngle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }

    // Degrees to radians
    inline float rad(float deg) { return deg * PI / 180.0f; }
    inline float deg(float rad) { return rad * 180.0f / PI; }

    // Clamp value
    template<typename T>
    inline T clamp(T val, T minVal, T maxVal) {
        return val < minVal ? minVal : (val > maxVal ? maxVal : val);
    }
}

#endif // SARATECH_H
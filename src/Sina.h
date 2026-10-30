#ifndef SARATECH_SINA_H
#define SARATECH_SINA_H

#include <Arduino.h>
#include "RobotState.h"
#include "MotorDriver.h"
#include "GTD35Adapter.h"

// Sina - Omnidirectional motion control
// Provides sina.motor(speed, x, y, tdaxisGain, acceleration)
// Supports both PWM (MotorDriver) and UART (GTD35Adapter) motor control

class Sina {
public:
    Sina() : _lastUpdate(0), _currentVx(0), _currentVy(0), _targetVx(0), _targetVy(0) {}

    void begin(MotorDriver* motorDriver = nullptr) {
        _motorDriver = motorDriver;
        _gtd35Adapter = nullptr;
        _lastUpdate = millis();
    }

    void begin(GTD35Adapter* gtd35Adapter) {
        _gtd35Adapter = gtd35Adapter;
        _motorDriver = nullptr;
        _lastUpdate = millis();
    }

    // Main movement interface
    // speed: requested linear speed in cm/s
    // x, y: direction vector components (normalized internally)
    // tdaxisGain: yaw correction gain multiplier (200 = original behavior, 0 = disabled)
    // acceleration: acceleration limit in cm/s² (0 = unlimited/instant)
    void motor(float speed, float x, float y, float tdaxisGain = 200.0f, float acceleration = 0.0f) {
        RobotState& state = RobotState::instance();

        // Normalize direction vector
        float magnitude = sqrt(x * x + y * y);
        float ux = 0, uy = 0;
        if (magnitude > 0.001f) {
            ux = x / magnitude;
            uy = y / magnitude;
        }

        // Calculate target velocity components (cm/s)
        _targetVx = speed * ux;
        _targetVy = speed * uy;

        // Apply acceleration limiting
        float dt = 0;
        uint32_t now = millis();
        if (_lastUpdate > 0) {
            dt = (now - _lastUpdate) / 1000.0f;
        }
        _lastUpdate = now;

        if (acceleration > 0 && dt > 0) {
            float maxDelta = acceleration * dt;
            float dvx = _targetVx - _currentVx;
            float dvy = _targetVy - _currentVy;

            if (fabs(dvx) > maxDelta) dvx = (dvx > 0) ? maxDelta : -maxDelta;
            if (fabs(dvy) > maxDelta) dvy = (dvy > 0) ? maxDelta : -maxDelta;

            _currentVx += dvx;
            _currentVy += dvy;
        } else {
            // Instant change (acceleration = 0 or first call)
            _currentVx = _targetVx;
            _currentVy = _targetVy;
        }

        // Convert cm/s to motor command units
        // For PWM: Max speed 30 cm/s = 65000 motor units
        // For GTD35: Max speed 30 cm/s = 32767 motor units (int16_t max)
        float maxMotorCmd = state.config.maxMotorCmd;
        if (_gtd35Adapter) {
            maxMotorCmd = 32767;  // GTD35 uses int16_t
        }

        float vxCmd = (_currentVx / state.config.maxSpeedCmS) * maxMotorCmd;
        float vyCmd = (_currentVy / state.config.maxSpeedCmS) * maxMotorCmd;

        // Clamp to max motor command
        if (fabs(vxCmd) > maxMotorCmd) vxCmd = (vxCmd > 0) ? maxMotorCmd : -maxMotorCmd;
        if (fabs(vyCmd) > maxMotorCmd) vyCmd = (vyCmd > 0) ? maxMotorCmd : -maxMotorCmd;

        // Inverse kinematics (original movexy)
        // Order: ML1, ML2, MR2, MR1
        float ml1 = vyCmd + vxCmd;
        float ml2 = vyCmd - vxCmd;
        float mr2 = -vyCmd - vxCmd;
        float mr1 = -vyCmd + vxCmd;

        // Yaw correction
        if (tdaxisGain > 0 && state.yawValid && !state.isYawStale()) {
            float yawError = state.yaw - state.dgy;  // gy - dgy
            // Normalize error to -180..180
            if (yawError > 180) yawError -= 360;
            if (yawError < -180) yawError += 360;

            // Clamp error
            if (yawError > 90) yawError = 90;
            if (yawError < -90) yawError = -90;

            // Apply gain: base 300 * (tdaxisGain / 200)
            float correction = yawError * state.config.yawGainBase * (tdaxisGain / 200.0f);

            // Apply same correction to all wheels for rotation
            // Note: For mecanum/omni, rotation is all wheels same sign
            ml1 += correction;
            ml2 += correction;
            mr2 += correction;
            mr1 += correction;
        } else if (tdaxisGain == 0) {
            // Yaw correction explicitly disabled
        } else {
            // Yaw data stale or invalid
            state.setError(RobotState::ERR_YAW_STALE);
        }

        // Send to motor driver
        if (_motorDriver) {
            _motorDriver->setMotorRaw(
                (int16_t)ml1, (int16_t)ml2, (int16_t)mr2, (int16_t)mr1
            );
        } else if (_gtd35Adapter) {
            _gtd35Adapter->setMotorRaw(
                (int16_t)ml1, (int16_t)ml2, (int16_t)mr2, (int16_t)mr1
            );
        }
    }

    // Stop all motion
    void stop() {
        _currentVx = 0;
        _currentVy = 0;
        _targetVx = 0;
        _targetVy = 0;
        if (_motorDriver) _motorDriver->stop();
        else if (_gtd35Adapter) _gtd35Adapter->stop();
    }

    // Get current velocity (cm/s)
    void getVelocity(float& vx, float& vy) const {
        vx = _currentVx;
        vy = _currentVy;
    }

    // Check if moving
    bool isMoving() const {
        return fabs(_currentVx) > 0.1f || fabs(_currentVy) > 0.1f;
    }

    // Set motor driver (can be called after construction)
    void setMotorDriver(MotorDriver* driver) {
        _motorDriver = driver;
        _gtd35Adapter = nullptr;
    }

    void setGTD35Adapter(GTD35Adapter* adapter) {
        _gtd35Adapter = adapter;
        _motorDriver = nullptr;
    }

    MotorDriver* getMotorDriver() const { return _motorDriver; }
    GTD35Adapter* getGTD35Adapter() const { return _gtd35Adapter; }

private:
    MotorDriver* _motorDriver = nullptr;
    GTD35Adapter* _gtd35Adapter = nullptr;
    uint32_t _lastUpdate = 0;
    float _currentVx, _currentVy;
    float _targetVx, _targetVy;
};

#endif // SARATECH_SINA_H
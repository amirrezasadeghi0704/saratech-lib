#ifndef SARATECH_SERVO_ADAPTER_H
#define SARATECH_SERVO_ADAPTER_H

#include <Arduino.h>
#include <Servo.h>
#include "RobotState.h"

// Servo adapter for shooter turret control
// Pin: PB10 (Timer 2 PWM)
// Usable range: 45° - 135° (center = 90° = forward)

class ServoAdapter {
public:
    ServoAdapter(uint8_t pin = PB10) : _pin(pin), _attached(false) {}

    void begin() {
        _servo.attach(_pin);
        _attached = true;
        setAngle(90.0f);  // Center position
        RobotState::instance().servoAttached = true;
        RobotState::instance().servoAngle = 90.0f;
    }

    void end() {
        if (_attached) {
            _servo.detach();
            _attached = false;
            RobotState::instance().servoAttached = false;
        }
    }

    // Set servo angle (degrees, 0-180)
    // Clamped to usable range 45-135
    bool setAngle(float angle) {
        if (!_attached) return false;

        RobotState& state = RobotState::instance();

        // Clamp to mechanical limits
        if (angle < state.config.servoMin) angle = state.config.servoMin;
        if (angle > state.config.servoMax) angle = state.config.servoMax;

        _servo.write((int)angle);
        state.servoAngle = angle;

        return true;
    }

    // Get current servo angle
    float getAngle() const {
        return RobotState::instance().servoAngle;
    }

    // Set angle relative to robot center (90°)
    // offset: degrees from center, positive = right
    bool setOffset(float offset) {
        RobotState& state = RobotState::instance();
        float target = state.config.servoCenter + offset;

        // Apply relative limit z (from smshoot)
        // This will be clamped by setAngle to servoMin/servoMax
        return setAngle(target);
    }

    // Check if angle is within usable range
    bool isInRange(float angle) const {
        RobotState& state = RobotState::instance();
        return angle >= state.config.servoMin && angle <= state.config.servoMax;
    }

    bool isAttached() const { return _attached; }
    uint8_t getPin() const { return _pin; }

private:
    Servo _servo;
    uint8_t _pin;
    bool _attached;
};

#endif // SARATECH_SERVO_ADAPTER_H
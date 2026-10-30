#ifndef SARATECH_MOTORDRIVER_H
#define SARATECH_MOTORDRIVER_H

#include <Arduino.h>
#include "RobotState.h"

// Motor pin mapping (matches original Secondry_ST_02.ino exactly)
struct MotorPins {
    // ML1 - Left Front
    uint8_t ml1Dir = PB15;
    uint8_t ml1Pwm = PB9;
    // ML2 - Left Rear
    uint8_t ml2Dir = PB14;
    uint8_t ml2Pwm = PB8;
    // MR2 - Right Rear
    uint8_t mr2Dir = PB13;
    uint8_t mr2Pwm = PB7;
    // MR1 - Right Front
    uint8_t mr1Dir = PB12;
    uint8_t mr1Pwm = PB6;
};

class MotorDriver {
public:
    MotorDriver(const MotorPins& pins = MotorPins()) : _pins(pins) {}

    void begin() {
        pinMode(_pins.ml1Dir, OUTPUT);
        pinMode(_pins.ml2Dir, OUTPUT);
        pinMode(_pins.mr2Dir, OUTPUT);
        pinMode(_pins.mr1Dir, OUTPUT);
        pinMode(_pins.ml1Pwm, OUTPUT);
        pinMode(_pins.ml2Pwm, OUTPUT);
        pinMode(_pins.mr2Pwm, OUTPUT);
        pinMode(_pins.mr1Pwm, OUTPUT);

        // Ensure all motors stopped initially
        setMotorRaw(0, 0, 0, 0);
    }

    // Set motor commands directly (raw PWM values, signed -32768 to 32767)
    // Order: ML1, ML2, MR2, MR1 (matches original motor() function)
    void setMotorRaw(int16_t ml1, int16_t ml2, int16_t mr2, int16_t mr1) {
        // Apply saturation at MAX_MOTOR level
        int16_t cmds[4] = {ml1, ml2, mr2, mr1};
        int maxAbs = 0;
        for (int i = 0; i < 4; i++) {
            if (abs(cmds[i]) > maxAbs) maxAbs = abs(cmds[i]);
        }

        RobotState& state = RobotState::instance();
        if (maxAbs > state.config.maxMotorCmd) {
            float scale = (float)state.config.maxMotorCmd / maxAbs;
            for (int i = 0; i < 4; i++) {
                cmds[i] = (int16_t)(cmds[i] * scale);
            }
        }

        // Clamp to 16-bit PWM range
        for (int i = 0; i < 4; i++) {
            if (cmds[i] > 32767) cmds[i] = 32767;
            if (cmds[i] < -32767) cmds[i] = -32767;
        }

        // Store for state tracking
        state.motorCmd[0] = cmds[0];
        state.motorCmd[1] = cmds[1];
        state.motorCmd[2] = cmds[2];
        state.motorCmd[3] = cmds[3];

        // Write to hardware (original encoding)
        writeMotor(_pins.ml1Dir, _pins.ml1Pwm, cmds[0]);
        writeMotor(_pins.ml2Dir, _pins.ml2Pwm, cmds[1]);
        writeMotor(_pins.mr2Dir, _pins.mr2Pwm, cmds[2]);
        writeMotor(_pins.mr1Dir, _pins.mr1Pwm, cmds[3]);
    }

    // Stop all motors
    void stop() {
        setMotorRaw(0, 0, 0, 0);
    }

    // Get current motor commands
    void getMotorCmd(int16_t (&cmds)[4]) const {
        RobotState& state = RobotState::instance();
        for (int i = 0; i < 4; i++) cmds[i] = state.motorCmd[i];
    }

private:
    MotorPins _pins;

    // Original motor encoding from Secondry_ST_02.ino
    // Positive: dir=0, pwm=value
    // Negative: dir=1, pwm=value+65535 (16-bit PWM)
    void writeMotor(uint8_t dirPin, uint8_t pwmPin, int16_t value) {
        if (value > 0) {
            digitalWrite(dirPin, 0);
            analogWrite(pwmPin, value);
        } else {
            digitalWrite(dirPin, 1);
            analogWrite(pwmPin, value + 65535);
        }
    }
};

#endif // SARATECH_MOTORDRIVER_H
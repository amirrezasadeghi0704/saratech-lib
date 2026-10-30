#ifndef SARATECH_GTD35_ADAPTER_H
#define SARATECH_GTD35_ADAPTER_H

#include <Arduino.h>
#include "GTD35.h"
#include "RobotState.h"

// Adapter that wraps the original GTD35 library for saratech integration
// Preserves original GTD35 implementation exactly as-is

class GTD35Adapter {
public:
    // Default constructor
    GTD35Adapter() : _gtd35(nullptr), _initialized(false) {}

    // Initialize with HardwareSerial reference (e.g., Serial1, Serial2, Serial3)
    void begin(HardwareSerial& serialPort, uint32_t baud = 115200, int8_t directionPin = -1, uint8_t txLevel = HIGH) {
        _gtd35 = new GTD35(serialPort);
        _gtd35->begin(baud, directionPin, txLevel);

        // Default motor IDs (0,1,2,3) - L1, L2, R2, R1 order
        _gtd35->setMotorIDs(0, 1, 2, 3);

        // Default angle (4000 as per original library)
        _gtd35->setAngle(4000);

        // Enable motors
        _gtd35->setEnabled(true);
        _gtd35->setResetEncoder(false);

        _initialized = true;
    }

    // Destructor
    ~GTD35Adapter() {
        if (_gtd35) {
            delete _gtd35;
        }
    }

    // Set motor commands using GTD35 protocol
    // Order: ML1, ML2, MR2, MR1 (matches original motor() function order)
    // Speeds are int16_t (-32768 to 32767) representing RPM
    void setMotorRaw(int16_t ml1, int16_t ml2, int16_t mr2, int16_t mr1) {
        if (!_initialized || !_gtd35) return;

        // Store for state tracking (convert to int16_t)
        RobotState& state = RobotState::instance();
        state.motorCmd[0] = ml1;
        state.motorCmd[1] = ml2;
        state.motorCmd[2] = mr2;
        state.motorCmd[3] = mr1;

        // Send via GTD35 protocol - order: L1, L2, R2, R1
        _gtd35->motor(ml1, ml2, mr2, mr1);
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

    // Configure motor IDs
    void setMotorIDs(uint8_t id0, uint8_t id1, uint8_t id2, uint8_t id3) {
        if (_gtd35) _gtd35->setMotorIDs(id0, id1, id2, id3);
    }

    // Set angle field sent to motors
    void setAngle(uint16_t angle) {
        if (_gtd35) _gtd35->setAngle(angle);
    }

    // Enable/disable motors
    void setEnabled(bool enabled) {
        if (_gtd35) _gtd35->setEnabled(enabled);
    }

    // Reset encoder
    void setResetEncoder(bool reset) {
        if (_gtd35) _gtd35->setResetEncoder(reset);
    }

    // Direct access to GTD35 for advanced features
    GTD35* getGTD35() { return _gtd35; }

    bool isInitialized() const { return _initialized; }

private:
    GTD35* _gtd35 = nullptr;
    bool _initialized = false;
};

#endif // SARATECH_GTD35_ADAPTER_H
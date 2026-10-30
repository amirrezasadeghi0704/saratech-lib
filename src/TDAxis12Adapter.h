#ifndef SARATECH_TDAXIS12_ADAPTER_H
#define SARATECH_TDAXIS12_ADAPTER_H

#include <Arduino.h>
#include <Wire.h>
#include "RobotState.h"

// Adapter that wraps TDAxis12 library functionality
// Preserves original API while integrating with RobotState

class TDAxis12Adapter {
public:
    // Constructor matches original TDAxis12
    TDAxis12Adapter(TwoWire* wire = &Wire, uint8_t address = 0x10)
        : _wire(wire), _address(address), _initialized(false) {}

    // Initialize I2C communication
    void begin(TwoWire* wire = nullptr, uint8_t address = 0xFF) {
        if (wire) _wire = wire;
        if (address != 0xFF) _address = address;

        _wire->begin();
        _initialized = true;
    }

    // Read yaw angle (degrees)
    // Original TDAxis12 returns negated angle: return -angle
    float read() {
        if (!_initialized) return 0.0f;

        _wire->beginTransmission(_address);
        _wire->write(4);  // Register address for angle data
        uint8_t result = _wire->endTransmission();
        if (result != 0) {
            RobotState::instance().setError(RobotState::ERR_YAW_STALE);
            return RobotState::instance().yaw;  // Return last known
        }

        _wire->requestFrom(_address, (uint8_t)18);
        uint8_t data[18];
        uint8_t count = 0;
        while (_wire->available() && count < 18) {
            data[count++] = _wire->read();
        }

        if (count < 2) {
            RobotState::instance().setError(RobotState::ERR_YAW_STALE);
            return RobotState::instance().yaw;
        }

        int16_t z = (int16_t)(data[0] << 8) | data[1];
        float angle = z / 100.0f;

        // Original TDAxis12 negates the angle
        float yaw = -angle;

        // Normalize to -180..180
        if (yaw > 180.0f) yaw -= 360.0f;
        if (yaw < -180.0f) yaw += 360.0f;

        RobotState::instance().updateYaw(yaw);
        RobotState::instance().clearError(RobotState::ERR_YAW_STALE);

        return yaw;
    }

        if (!_initialized) return false;

        _wire->beginTransmission(_address);
        _wire->write(4);
        if (_wire->endTransmission() != 0) return false;

        _wire->requestFrom(_address, (uint8_t)18);
        uint8_t data[18];
        uint8_t count = 0;
        while (_wire->available() && count < 18) {
            data[count++] = _wire->read();
        }

        if (count < 18) return false;

        return true;
    }

    // Check if ball detected (matches original is_ball logic)
    bool isBallDetected() {
        uint8_t sensors[16];
        if (!readSensors(sensors)) return false;

        float sumx = 0, sumy = 0;
            float sensorAngle = radians(i * 22.5f);
            float value = 4095.0f - sensors[i];  // Inverted reading
            sumx += value * cos(sensorAngle);
            sumy += value * sin(sensorAngle);
     

        float ballAngle = degrees(atan2(sumy, sumx));
        float ballDistance = sqrt(sumx * sumx + sumy * sumy);

        // Original threshold: balldistance > 2000
        return ballDistance > 2000.0f;
    }

    // Get full ball data
    bool getBallData(float& outAngle, float& outDistance) {
        uint8_t sensors[16];
        if (!readSensors(sensors)) return false;

        float sumx = 0, sumy = 0;
        for (int i = 0; i < 16; i++) {
            float sensorAngle = radians(i * 22.5f);
            float value = 4095.0f - sensors[i];
            sumx += value * cos(sensorAngle);
            sumy += value * sin(sensorAngle);
        }

        outAngle = degrees(atan2(sumy, sumx));
        outDistance = sqrt(sumx * sumx + sumy * sumy);

        // Normalize angle
        if (outAngle > 180.0f) outAngle -= 360.0f;
        if (outAngle < -180.0f) outAngle += 360.0f;

        return true;
    }

    bool isInitialized() const { return _initialized; }
    uint8_t getAddress() const { return _address; }

private:
    TwoWire* _wire;
    uint8_t _address;
    bool _initialized;
};

#endif // SARATECH_TDAXIS12_ADAPTER_H

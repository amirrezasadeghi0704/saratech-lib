#ifndef SARATECH_ROBOTSTATE_H
#define SARATECH_ROBOTSTATE_H

#include <Arduino.h>

class RobotState {
public:
    // Singleton access
    static RobotState& instance() {
        static RobotState inst;
        return inst;
    }

    // Yaw (degrees, TDAxis12 convention: 0=forward, positive=clockwise)
    float yaw = 0.0f;
    bool yawValid = false;
    uint32_t yawTimestamp = 0;

    // Position (field coordinates, cm, origin at our goal bottom-left)
    float posX = 0.0f;
    float posY = 0.0f;
    bool posValid = false;
    uint32_t posTimestamp = 0;

    // Distance sensors (cm, calibrated)
    float distFront = 0.0f;
    float distRight = 0.0f;
    float distBack = 0.0f;
    float distLeft = 0.0f;
    bool distValid[4] = {false, false, false, false};
    uint32_t distTimestamp = 0;

    // Ball detection (TSOP array)
    float ballAngle = 0.0f;        // Robot-centric degrees, 0=forward, +right
    float ballDistance = 0.0f;     // Arbitrary units
    bool ballValid = false;
    bool ballInKicker = false;
    uint32_t ballTimestamp = 0;

    // Side sensors for yaw offset (dgy)
    int shr = 0;
    int shl = 0;
    int shb = 0;
    float dgy = 0.0f;              // Desired yaw offset from side sensors

    // Shooter
    bool shooterReady = false;     // PA4 active LOW = ready
    bool shooterFiring = false;
    uint32_t shooterPulseStart = 0;
    const uint32_t SHOOTER_PULSE_MS = 200;

    // Servo
    float servoAngle = 90.0f;      // 90 = center (forward), usable 45-135
    bool servoAttached = false;

    // Movement targets
    float targetX = 0.0f;
    float targetY = 0.0f;
    bool hasTarget = false;

    // Motor commands (raw PWM values, signed)
    int16_t motorCmd[4] = {0, 0, 0, 0};  // ML1, ML2, MR2, MR1

    // Error/status flags
    uint8_t errorFlags = 0;
    enum ErrorFlag {
        ERR_YAW_STALE = 1 << 0,
        ERR_POS_INVALID = 1 << 1,
        ERR_SENSOR_INVALID = 1 << 2,
        ERR_SHOOTER_NOT_READY = 1 << 3,
        ERR_SERVO_LIMIT = 1 << 4,
        ERR_MOVEMENT_TIMEOUT = 1 << 5
    };

    // Configuration (set during init)
    struct Config {
        // Motor
        float maxSpeedCmS = 30.0f;
        int maxMotorCmd = 65000;
        float yawGainBase = 300.0f;  // motor units per degree

        // Field
        float fieldWidth = 180.0f;   // X: 0-180
        float fieldLength = 240.0f;  // Y: 0-240

        // Arrival tolerance
        float arrivalTolerance = 5.0f;  // cm

        // Shooting
        float shootAlignmentTol = 3.0f;  // degrees
        float servoCenter = 90.0f;
        float servoMin = 45.0f;
        float servoMax = 135.0f;

        // Sensor calibration
        float adcToCmFactor = 0.1f;  // (reading - baseline) * factor = cm

        // Timing
        uint32_t maxYawAge = 100;     // ms
        uint32_t maxPosAge = 500;     // ms
        uint32_t movementTimeout = 5000;  // ms
    } config;

    // Distance sensor baselines (set at startup)
    int baselineFront = 0;
    int baselineRight = 0;
    int baselineBack = 0;
    int baselineLeft = 0;
    bool baselinesSet = false;

private:
    RobotState() = default;
    RobotState(const RobotState&) = delete;
    RobotState& operator=(const RobotState&) = delete;

public:
    // Update yaw from TDAxis12
    void updateYaw(float newYaw) {
        yaw = newYaw;
        yawValid = true;
        yawTimestamp = millis();
    }

    // Update position estimate
    void updatePosition(float x, float y, bool valid) {
        posX = x;
        posY = y;
        posValid = valid;
        posTimestamp = millis();
    }

    // Update distances
    void updateDistances(float front, float right, float back, float left,
                         bool vFront, bool vRight, bool vBack, bool vLeft) {
        distFront = front;
        distRight = right;
        distBack = back;
        distLeft = left;
        distValid[0] = vFront;
        distValid[1] = vRight;
        distValid[2] = vBack;
        distValid[3] = vLeft;
        distTimestamp = millis();
    }

    // Update ball detection
    void updateBall(float angle, float distance, bool valid, bool inKicker) {
        ballAngle = angle;
        ballDistance = distance;
        ballValid = valid;
        ballInKicker = inKicker;
        ballTimestamp = millis();
    }

    // Update side sensors
    void updateSideSensors(int shrVal, int shlVal, int shbVal) {
        shr = shrVal;
        shl = shlVal;
        shb = shbVal;
        // Calculate dgy (desired yaw offset) - matches original logic
        if (shr > 900 && ballValid) dgy = 21.80f;
        else if (shl > 1100 && ballValid) dgy = -21.00f;
        else if (!ballValid || abs(shl - shr) < 6000) dgy = 0.0f;
    }

    // Check if data is stale
    bool isYawStale() const { return (millis() - yawTimestamp) > config.maxYawAge; }
    bool isPosStale() const { return (millis() - posTimestamp) > config.maxPosAge; }

    // Clear error flag
    void clearError(ErrorFlag flag) { errorFlags &= ~flag; }
    void setError(ErrorFlag flag) { errorFlags |= flag; }
    bool hasError(ErrorFlag flag) const { return errorFlags & flag; }
    void clearAllErrors() { errorFlags = 0; }

    // Set baselines
    void setBaselines(int f, int r, int b, int l) {
        baselineFront = f;
        baselineRight = r;
        baselineBack = b;
        baselineLeft = l;
        baselinesSet = true;
    }

    // Convert raw ADC to cm using calibration
    float adcToCm(int raw, int baseline) const {
        if (!baselinesSet) return 0.0f;
        return (raw - baseline) * config.adcToCmFactor;
    }
};

#endif // SARATECH_ROBOTSTATE_H
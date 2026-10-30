/**
 * @file saratech.h
 * @brief Saratech Soccer Robot Library - Open Weight League
 * @version 2.0.0
 * @author Saratech Team (amirreza, sadra, sina, king.forati, sina)
 *
 * Consolidated library with:
 * - All TDaxis12 commands preserved
 * - All GTD35 commands preserved
 * - New classes: amirreza_class, sina_class, sadra_class, king.forati_class
 * - Wheel radius: 5 cm
 * - Components: spin back motor, GTD35, TDAxis, Servo
 */

#ifndef SARATECH_H
#define SARATECH_H

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>

// ============================================================================
// INTERNAL COMPONENTS (4 allowed components only)
// ============================================================================

// --- Robot State Singleton ---
class RobotState {
public:
    static RobotState& instance() {
        static RobotState inst;
        return inst;
    }

    float yaw = 0.0f;
    bool yawValid = false;
    uint32_t yawTimestamp = 0;
    float dgy = 0.0f;

    float posX = 0.0f, posY = 0.0f;
    bool posValid = false;
    uint32_t posTimestamp = 0;

    float distFront = 0, distRight = 0, distBack = 0, distLeft = 0;
    bool distValid[4] = {false, false, false, false};
    uint32_t distTimestamp = 0;

    float ballAngle = 0, ballDistance = 0;
    bool ballValid = false, ballInKicker = false;
    uint32_t ballTimestamp = 0;

    int shr = 0, shl = 0, shb = 0;

    bool shooterReady = false, shooterFiring = false;
    uint32_t shooterPulseStart = 0;
    const uint32_t SHOOTER_PULSE_MS = 200;

    float servoAngle = 90.0f;
    bool servoAttached = false;

    float targetX = 0, targetY = 0;
    bool hasTarget = false;

    int16_t motorCmd[4] = {0, 0, 0, 0};

    uint8_t errorFlags = 0;
    enum ErrorFlag {
        ERR_YAW_STALE = 1 << 0,
        ERR_POS_INVALID = 1 << 1,
        ERR_SENSOR_INVALID = 1 << 2,
        ERR_SHOOTER_NOT_READY = 1 << 3,
        ERR_SERVO_LIMIT = 1 << 4,
        ERR_MOVEMENT_TIMEOUT = 1 << 5
    };

    struct Config {
        float maxSpeedCmS = 30.0f;
        int maxMotorCmd = 65000;
        float yawGainBase = 300.0f;
        float fieldWidth = 180.0f;
        float fieldLength = 240.0f;
        float arrivalTolerance = 5.0f;
        float shootAlignmentTol = 3.0f;
        float servoCenter = 90.0f;
        float servoMin = 45.0f;
        float servoMax = 135.0f;
        float adcToCmFactor = 0.1f;
        uint32_t maxYawAge = 100;
        uint32_t maxPosAge = 500;
        uint32_t movementTimeout = 5000;
    } config;

    int baselineFront = 0, baselineRight = 0, baselineBack = 0, baselineLeft = 0;
    bool baselinesSet = false;

private:
    RobotState() = default;

public:
    void updateYaw(float newYaw) {
        yaw = newYaw;
        yawValid = true;
        yawTimestamp = millis();
    }

    void updatePosition(float x, float y, bool valid) {
        posX = x; posY = y;
        posValid = valid; posTimestamp = millis();
    }

    void updateDistances(float front, float right, float back, float left,
                         bool vFront, bool vRight, bool vBack, bool vLeft) {
        distFront = front; distRight = right;
        distBack = back; distLeft = left;
        distValid[0] = vFront; distValid[1] = vRight;
        distValid[2] = vBack; distValid[3] = vLeft;
        distTimestamp = millis();
    }

    void updateBall(float angle, float distance, bool valid, bool inKicker) {
        ballAngle = angle; ballDistance = distance;
        ballValid = valid; ballInKicker = inKicker;
        ballTimestamp = millis();
    }

    void updateSideSensors(int shrVal, int shlVal, int shbVal) {
        shr = shrVal; shl = shlVal; shb = shbVal;
        // Original dgy logic
        if (shr > 900 && ballValid) dgy = 21.80f;
        else if (shl > 1100 && ballValid) dgy = -21.00f;
        else if (!ballValid || abs(shl - shr) < 6000) dgy = 0.0f;
    }

    bool isYawStale() const { return (millis() - yawTimestamp) > config.maxYawAge; }
    bool isPosStale() const { return (millis() - posTimestamp) > config.maxPosAge; }

    void clearError(ErrorFlag flag) { errorFlags &= ~flag; }
    void setError(ErrorFlag flag) { errorFlags |= flag; }
    bool hasError(ErrorFlag flag) const { return errorFlags & flag; }
    void clearAllErrors() { errorFlags = 0; }

    void setBaselines(int f, int r, int b, int l) {
        baselineFront = f; baselineRight = r;
        baselineBack = b; baselineLeft = l;
        baselinesSet = true;
    }

    float adcToCm(int raw, int baseline) const {
        if (!baselinesSet) return 0.0f;
        return (raw - baseline) * config.adcToCmFactor;
    }

    // Ball/sensor data
    float ballAngle = 0, ballDistance = 0;
    bool ballValid = false, ballInKicker = false;
    uint32_t ballTimestamp = 0;

    float distFront = 0, distRight = 0, distBack = 0, distLeft = 0;
    bool distValid[4] = {false, false, false, false};
};

// ============================================================================
// COMPONENT 1: TDAXIS12 ADAPTER (preserved original)
// ============================================================================

class TDAxis12Adapter {
public:
    TDAxis12Adapter(TwoWire* wire = &Wire, uint8_t address = 0x10)
        : _wire(wire), _address(address), _initialized(false) {}

    void begin(TwoWire* wire = nullptr, uint8_t address = 0xFF) {
        if (wire) _wire = wire;
        if (address != 0xFF) _address = address;
        _wire->begin();
        _initialized = true;
    }

    float read() {
        if (!_initialized) return 0.0f;
        _wire->beginTransmission(_address);
        _wire->write(4);
        if (_wire->endTransmission() != 0) {
            RobotState::instance().setError(RobotState::ERR_YAW_STALE);
            return RobotState::instance().yaw;
        }
        _wire->requestFrom(_address, (uint8_t)18);
        uint8_t data[18];
        uint8_t count = 0;
        while (_wire->available() && count < 18) data[count++] = _wire->read();
        if (count < 2) {
            RobotState::instance().setError(RobotState::ERR_YAW_STALE);
            return RobotState::instance().yaw;
        }
        int16_t z = (int16_t)(data[0] << 8) | data[1];
        float angle = z / 100.0f;
        float yaw = -angle;
        if (yaw > 180.0f) yaw -= 360.0f;
        if (yaw < -180.0f) yaw += 360.0f;
        RobotState::instance().updateYaw(yaw);
        RobotState::instance().clearError(RobotState::ERR_YAW_STALE);
        return yaw;
    }

    bool readSensors(uint8_t (&sensors)[16]) {
        if (!_initialized) return false;
        _wire->beginTransmission(_address);
        _wire->write(4);
        if (_wire->endTransmission() != 0) return false;
        _wire->requestFrom(_address, (uint8_t)18);
        uint8_t data[18];
        uint8_t count = 0;
        while (_wire->available() && count < 18) data[count++] = _wire->read();
        if (count < 18) return false;
        for (int i = 0; i < 16; i++) sensors[i] = data[2 + i];
        return true;
    }

    bool isBallDetected() {
        uint8_t sensors[16];
        if (!readSensors(sensors)) return false;
        float sumx = 0, sumy = 0;
        for (int i = 0; i < 16; i++) {
            float sensorAngle = radians(i * 22.5f);
            float value = 4095.0f - sensors[i];
            sumx += value * cos(sensorAngle);
            sumy += value * sin(sensorAngle);
        }
        float ballDistance = sqrt(sumx * sumx + sumy * sumy);
        return ballDistance > 2000.0f;
    }

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

// ============================================================================
// COMPONENT 2: GTD35 (preserved original)
// ============================================================================

class GTD35 {
public:
    explicit GTD35(HardwareSerial& serialPort)
        : _serial(serialPort), _directionPin(-1), _txLevel(HIGH),
          _angle(4000), _enabled(true), _resetEncoder(false) {
        _motorIDs[0] = 0; _motorIDs[1] = 1;
        _motorIDs[2] = 2; _motorIDs[3] = 3;
    }

    void begin(uint32_t baud = 115200, int8_t directionPin = -1, uint8_t txLevel = HIGH) {
        _directionPin = directionPin; _txLevel = txLevel;
        if (_directionPin >= 0) {
            pinMode(_directionPin, OUTPUT);
            digitalWrite(_directionPin, !_txLevel);
        }
        _serial.begin(baud);
    }

    uint8_t checksum(const uint8_t* data, size_t length) {
        uint8_t result = 0;
        for (size_t i = 0; i < length; ++i) result ^= data[i];
        return result;
    }

    void setMotorIDs(uint8_t id0, uint8_t id1, uint8_t id2, uint8_t id3) {
        _motorIDs[0] = id0; _motorIDs[1] = id1;
        _motorIDs[2] = id2; _motorIDs[3] = id3;
    }

    void setAngle(uint16_t angle) { _angle = angle; }
    void setEnabled(bool enabled) { _enabled = enabled; }
    void setResetEncoder(bool reset) { _resetEncoder = reset; }

    bool motor(int16_t v0, int16_t v1, int16_t v2, int16_t v3) {
        const int16_t speeds[4] = {v0, v1, v2, v3};
        uint8_t frame[2 + 1 + 4 + (4 * 5) + 1];
        size_t pos = 0;
        frame[pos++] = 0xAA; frame[pos++] = 0xAA; frame[pos++] = 4;
        for (uint8_t i = 0; i < 4; ++i) frame[pos++] = _motorIDs[i];
        const uint8_t control = (_enabled ? 0x0F : 0x00) | (_resetEncoder ? 0xF0 : 0x00);
        for (uint8_t i = 0; i < 4; ++i) {
            const uint16_t speedBits = static_cast<uint16_t>(speeds[i]);
            frame[pos++] = control;
            frame[pos++] = static_cast<uint8_t>((speedBits >> 8) & 0xFF);
            frame[pos++] = static_cast<uint8_t>(speedBits & 0xFF);
            frame[pos++] = static_cast<uint8_t>((_angle >> 8) & 0xFF);
            frame[pos++] = static_cast<uint8_t>(_angle & 0xFF);
        }
        frame[pos] = checksum(&frame[2], pos - 2); ++pos;
        sendFrame(frame, pos);
        return true;
    }

    bool setMotorID(uint8_t previousID, uint8_t newID) {
        uint8_t frame[7] = {0xAA, 0xAA, 0x00, previousID, 0x11, newID, 0x00};
        frame[6] = checksum(&frame[2], 4);
        sendFrame(frame, sizeof(frame));
        return true;
    }

    void setID(uint8_t newID) {
        for (uint16_t previousID = 0; previousID < 255; previousID++) {
            if (previousID == newID) continue;
            setMotorID((uint8_t)previousID, newID);
            delay(5);
        }
    }

private:
    HardwareSerial& _serial;
    int8_t _directionPin; uint8_t _txLevel;
    uint16_t _angle; bool _enabled; bool _resetEncoder;
    uint8_t _motorIDs[4];

    void sendFrame(const uint8_t* data, size_t length) {
        if (_directionPin >= 0) {
            digitalWrite(_directionPin, _txLevel);
            delayMicroseconds(2);
        }
        _serial.write(data, length); _serial.flush();
        if (_directionPin >= 0) {
            delayMicroseconds(2);
            digitalWrite(_directionPin, !_txLevel);
        }
    }
};

// GTD35 Adapter
class GTD35Adapter {
public:
    GTD35Adapter() : _gtd35(nullptr), _initialized(false) {}

    void begin(HardwareSerial& serialPort, uint32_t baud = 115200,
               int8_t directionPin = -1, uint8_t txLevel = HIGH) {
        _gtd35 = new GTD35(serialPort);
        _gtd35->begin(baud, directionPin, txLevel);
        _gtd35->setMotorIDs(0, 1, 2, 3);
        _gtd35->setAngle(4000);
        _gtd35->setEnabled(true);
        _gtd35->setResetEncoder(false);
        _initialized = true;
    }

    ~GTD35Adapter() { if (_gtd35) delete _gtd35; }

    void setMotorRaw(int16_t ml1, int16_t ml2, int16_t mr2, int16_t mr1) {
        if (!_initialized || !_gtd35) return;
        RobotState& state = RobotState::instance();
        state.motorCmd[0] = ml1; state.motorCmd[1] = ml2;
        state.motorCmd[2] = mr2; state.motorCmd[3] = mr1;
        _gtd35->motor(ml1, ml2, mr2, mr1);
    }

    void stop() { setMotorRaw(0, 0, 0, 0); }

    void getMotorCmd(int16_t (&cmds)[4]) const {
        RobotState& state = RobotState::instance();
        for (int i = 0; i < 4; i++) cmds[i] = state.motorCmd[i];
    }

    void setMotorIDs(uint8_t id0, uint8_t id1, uint8_t id2, uint8_t id3) {
        if (_gtd35) _gtd35->setMotorIDs(id0, id1, id2, id3);
    }
    void setAngle(uint16_t angle) { if (_gtd35) _gtd35->setAngle(angle); }
    void setEnabled(bool enabled) { if (_gtd35) _gtd35->setEnabled(enabled); }
    void setResetEncoder(bool reset) { if (_gtd35) _gtd35->setResetEncoder(reset); }
    GTD35* getGTD35() { return _gtd35; }
    bool isInitialized() const { return _initialized; }

private:
    GTD35* _gtd35 = nullptr;
    bool _initialized = false;
};

// ============================================================================
// COMPONENT 3: SERVO ADAPTER (copy of Servo.h functionality)
// ============================================================================

class ServoAdapter {
public:
    ServoAdapter(uint8_t pin = PB10) : _pin(pin), _attached(false) {}

    void begin() {
        _servo.attach(_pin);
        _attached = true;
        setAngle(90.0f);
        RobotState::instance().servoAttached = true;
        RobotState::instance().servoAngle = 90.0f;
    }

    void end() {
        if (_attached) { _servo.detach(); _attached = false; }
        RobotState::instance().servoAttached = false;
    }

    bool setAngle(float angle) {
        if (!_attached) return false;
        RobotState& state = RobotState::instance();
        if (angle < state.config.servoMin) angle = state.config.servoMin;
        if (angle > state.config.servoMax) angle = state.config.servoMax;
        _servo.write((int)angle);
        state.servoAngle = angle;
        return true;
    }

    float getAngle() const { return RobotState::instance().servoAngle; }

    bool setOffset(float offset) {
        RobotState& state = RobotState::instance();
        float target = state.config.servoCenter + offset;
        return setAngle(target);
    }

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

// ============================================================================
// COMPONENT 4: SPIN BACK MOTOR
// ============================================================================

class SpinBackMotor {
public:
    SpinBackMotor(uint8_t pwmPin = PB11, uint8_t dirPin = PB12)
        : _pwmPin(pwmPin), _dirPin(dirPin) {}

    void begin() {
        pinMode(_pwmPin, OUTPUT);
        pinMode(_dirPin, OUTPUT);
        analogWrite(_pwmPin, 0);
        digitalWrite(_dirPin, 0);
    }

    // Speed: -255 to 255 (or -65535 to 65535 for 16-bit PWM)
    void setSpeed(int16_t speed) {
        if (speed > 0) {
            digitalWrite(_dirPin, 0);
            analogWrite(_pwmPin, speed);
        } else {
            digitalWrite(_dirPin, 1);
            analogWrite(_pwmPin, speed + 65535);
        }
    }

    void stop() { setSpeed(0); }

private:
    uint8_t _pwmPin, _dirPin;
};

// ============================================================================
// NEW CLASS 1: amirreza_class (TDAxis coefficient receiver)
// ============================================================================

class amirreza_class {
public:
    amirreza_class() : _tdaxisGain(200.0f), _useYawCorrection(true) {}

    // 0-Receive TDAxis coefficient for all movement functions
    void setTdaxisGain(float gain) { _tdaxisGain = gain; }
    float getTdaxisGain() const { return _tdaxisGain; }

    void enableYawCorrection(bool enable) { _useYawCorrection = enable; }
    bool isYawCorrectionEnabled() const { return _useYawCorrection; }

    // Apply yaw correction to motor commands
    // Returns corrected motor commands (ml1, ml2, mr2, mr1)
    void applyYawCorrection(int16_t (&motorCmds)[4], float currentYaw, float targetYaw) {
        if (!_useYawCorrection) return;

        RobotState& state = RobotState::instance();
        if (!state.yawValid || state.isYawStale()) {
            state.setError(RobotState::ERR_YAW_STALE);
            return;
        }

        float yawError = targetYaw - currentYaw;
        // Normalize to -180..180
        while (yawError > 180) yawError -= 360;
        while (yawError < -180) yawError += 360;

        if (yawError > 90) yawError = 90;
        if (yawError < -90) yawError = -90;

        // Correction = yawError * baseGain * (gain/200)
        RobotState& state = RobotState::instance();
        float correction = yawError * state.config.yawGainBase * (_tdaxisGain / 200.0f);

        // Apply same correction to all wheels for rotation
        for (int i = 0; i < 4; i++) {
            motorCmds[i] += (int16_t)correction;
        }
    }

private:
    float _tdaxisGain;
    bool _useYawCorrection;
};

// ============================================================================
// NEW CLASS 2: sina_class (Robot control with cm/s + vector + acceleration)
// ============================================================================

class sina_class {
public:
    sina_class(GTD35Adapter* gtd35 = nullptr, SpinBackMotor* spinMotor = nullptr)
        : _gtd35(gtd35), _spinMotor(spinMotor),
          _currentVx(0), _currentVy(0), _targetVx(0), _targetVy(0),
          _lastUpdate(0) {}

    void begin(GTD35Adapter* gtd35 = nullptr, SpinBackMotor* spinMotor = nullptr) {
        _gtd35 = gtd35; _spinMotor = spinMotor;
        _lastUpdate = millis();
    }

    // 1- Control robot with cm/s and vector (3 inputs: speed, x, y)
    // 1.1- Acceleration limit (1 input: acceleration cm/s²)
    void motor(float speed_cm_s, float x, float y,
               float tdaxisGain = 200.0f, float acceleration_cm_s2 = 0.0f) {
        RobotState& state = RobotState::instance();

        // Normalize direction vector
        float magnitude = sqrt(x * x + y * y);
        float ux = 0, uy = 0;
        if (magnitude > 0.001f) {
            ux = x / magnitude;
            uy = y / magnitude;
        }

        // Target velocity in cm/s
        _targetVx = speed_cm_s * ux;
        _targetVy = speed_cm_s * uy;

        // Acceleration limiting
        float dt = 0;
        uint32_t now = millis();
        if (_lastUpdate > 0) dt = (now - _lastUpdate) / 1000.0f;
        _lastUpdate = now;

        if (acceleration_cm_s2 > 0 && dt > 0) {
            float maxDelta = acceleration_cm_s2 * dt;
            float dvx = _targetVx - _currentVx;
            float dvy = _targetVy - _currentVy;

            if (fabs(dvx) > maxDelta) dvx = (dvx > 0) ? maxDelta : -maxDelta;
            if (fabs(dvy) > maxDelta) dvy = (dvy > 0) ? maxDelta : -maxDelta;

            _currentVx += dvx;
            _currentVy += dvy;
        } else {
            _currentVx = _targetVx;
            _currentVy = _targetVy;
        }

        // Convert cm/s to motor command units
        RobotState& state = RobotState::instance();
        float maxMotorCmd = state.config.maxMotorCmd;
        if (_gtd35 && _gtd35->isInitialized()) maxMotorCmd = 32767;

        float vxCmd = (_currentVx / state.config.maxSpeedCmS) * maxMotorCmd;
        float vyCmd = (_currentVy / state.config.maxSpeedCmS) * maxMotorCmd;

        if (fabs(vxCmd) > maxMotorCmd) vxCmd = (vxCmd > 0) ? maxMotorCmd : -maxMotorCmd;
        if (fabs(vyCmd) > maxMotorCmd) vyCmd = (vyCmd > 0) ? maxMotorCmd : -maxMotorCmd;

        // Inverse kinematics (original movexy order: ML1, ML2, MR2, MR1)
        float ml1 = vyCmd + vxCmd;
        float ml2 = vyCmd - vxCmd;
        float mr2 = -vyCmd - vxCmd;
        float mr1 = -vyCmd + vxCmd;

        // Yaw correction using amirreza_class logic
        if (state.yawValid && !state.isYawStale()) {
            float yawError = state.yaw - state.dgy;
            while (yawError > 180) yawError -= 360;
            while (yawError < -180) yawError += 360;
            if (yawError > 90) yawError = 90;
            if (yawError < -90) yawError = -90;

            float correction = yawError * state.config.yawGainBase * (200.0f / 200.0f);

            ml1 += correction; ml2 += correction;
            mr2 += correction; mr1 += correction;
        }

        int16_t cmds[4] = {(int16_t)ml1, (int16_t)ml2, (int16_t)mr2, (int16_t)mr1};

        // Apply TDAxis correction from amirreza_class
        _amirreza.applyYawCorrection(cmds, RobotState::instance().yaw, 0);

        // Send to motor driver
        if (_gtd35 && _gtd35->isInitialized()) {
            _gtd35->setMotorRaw((int16_t)cmds[0], (int16_t)cmds[1],
                                (int16_t)cmds[2], (int16_t)cmds[3]);
        }
    }

    void stop() {
        _currentVx = _currentVy = _targetVx = _targetVy = 0;
        if (_gtd35 && _gtd35->isInitialized()) _gtd35->stop();
    }

    void getVelocity(float& vx, float& vy) const { vx = _currentVx; vy = _currentVy; }
    bool isMoving() const { return fabs(_currentVx) > 0.1f || fabs(_currentVy) > 0.1f; }

private:
    GTD35Adapter* _gtd35 = nullptr;
    SpinBackMotor* _spinMotor = nullptr;
    amirreza_class _amirreza;
    uint32_t _lastUpdate = 0;
    float _currentVx, _currentVy, _targetVx, _targetVy;
};

// ============================================================================
// NEW CLASS 3: sadra_class (Go to point with cm/s - 3 inputs)
// ============================================================================

class sadra_class {
public:
    sadra_class(sina_class* sina = nullptr) : _sina(sina) {}

    void begin(sina_class* sina = nullptr) { _sina = sina; }

    // 2- Go to point with cm/s (3 inputs: targetX, targetY, speed_cm_s)
    // Returns true when target reached
    bool move(int targetX_cm, int targetY_cm, int speed_cm_s) {
        RobotState& state = RobotState::instance();

        if (!state.posValid || state.isPosStale()) {
            state.setError(RobotState::ERR_POS_INVALID);
            return false;
        }

        state.targetX = targetX_cm;
        state.targetY = targetY_cm;
        state.hasTarget = true;

        float dx = targetX_cm - state.posX;
        float dy = targetY_cm - state.posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= state.config.arrivalTolerance) {
            if (_sina) _sina->stop();
            state.hasTarget = false;
            return true;
        }

        float dirX = dx / distance;
        float dirY = dy / distance;

        // Speed reduction when close
        float approachSpeed = speed_cm_s;
        if (distance < 30.0f) {
            approachSpeed = max(5.0f, speed_cm_s * (distance / 30.0f));
        }

        if (_sina) {
            _sina->motor(approachSpeed, dirX * 100, dirY * 100, 200, 0);
        }

        return false;
    }

    bool isMoving() const {
        return RobotState::instance().hasTarget;
    }

    void stop() {
        RobotState::instance().hasTarget = false;
        if (_sina) _sina->stop();
    }

    float distanceToTarget() const {
        RobotState& state = RobotState::instance();
        if (!state.hasTarget) return 0;
        float dx = state.targetX - state.posX;
        float dy = state.targetY - state.posY;
        return sqrt(dx * dx + dy * dy);
    }

private:
    sina_class* _sina = nullptr;
};

// ============================================================================
// NEW CLASS 4: king.forati_class (Return to start - 3 inputs)
// ============================================================================

class king_forati_class {
public:
    king_forati_class(sina_class* sina = nullptr) : _sina(sina) {}

    void begin(sina_class* sina = nullptr) { _sina = sina; }

    // 3- Return to start point (3 inputs: speed, xtarg, ytarg)
    void forati(int speed_cm_s, int xtarg_cm, int ytarg_cm) {
        RobotState& state = RobotState::instance();

        if (!state.posValid || state.isPosStale()) {
            state.setError(RobotState::ERR_POS_INVALID);
            if (_sina) _sina->stop();
            return;
        }

        float dx = xtarg_cm - state.posX;
        float dy = ytarg_cm - state.posY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance <= state.config.arrivalTolerance) {
            if (_sina) _sina->stop();
            return;
        }

        float dirX = dx / distance;
        float dirY = dy / distance;

        // Distance-proportional speed
        float kd = (float)speed_cm_s / 50.0f;
        float targetSpeed = min((float)speed_cm_s, kd * distance);
        targetSpeed = max(targetSpeed, 5.0f);

        if (_sina) {
            _sina->motor(targetSpeed, dirX * 100, dirY * 100, 200, 0);
        }
    }

private:
    sina_class* _sina = nullptr;
};

// ============================================================================
// NEW CLASS 5: sina_class rotation (2 inputs: manual gyro addition + servo)
// ============================================================================

// Note: This is part of sina_class but exposing rotation-specific interface
class sina_rotation_class {
public:
    sina_rotation_class(sina_class* sina = nullptr, ServoAdapter* servo = nullptr)
        : _sina(sina), _servo(servo) {}

    void begin(sina_class* sina = nullptr, ServoAdapter* servo = nullptr) {
        _sina = sina; _servo = servo;
    }

    // 4- Rotation with manual gyro addition + servo (2 inputs)
    // Input 1: manual_gyro_addition (added to current gyro before multiplying by coefficient)
    // Input 2: servo_angle (for turret aiming)
    void rotate(float manual_gyro_addition, float servo_angle) {
        RobotState& state = RobotState::instance();

        // Effective yaw = current yaw + manual addition
        float effectiveYaw = state.yaw + manual_gyro_addition;

        // For in-place rotation: all wheels same direction
        // Target: rotate to reduce yaw error
        float yawError = effectiveYaw - state.dgy;
        while (yawError > 180) yawError -= 360;
        while (yawError < -180) yawError += 360;

        if (yawError > 90) yawError = 90;
        if (yawError < -90) yawError = -90;

        float correction = yawError * state.config.yawGainBase * (200.0f / 200.0f);
        int16_t rotCmd = (int16_t)correction;

        if (_gtd35 && _gtd35->isInitialized()) {
            _gtd35->setMotorRaw(rotCmd, rotCmd, rotCmd, rotCmd);
        }

        // Servo control (Input 2)
        if (_servo && _servo->isAttached()) {
            float targetAngle = state.config.servoCenter + servo_angle;
            _servo->setAngle(targetAngle);
        }
    }

    void stop() {
        if (_gtd35 && _gtd35->isInitialized()) _gtd35->stop();
    }

    void setGTD35(GTD35Adapter* gtd35) { _gtd35 = gtd35; }
    void setServo(ServoAdapter* servo) { _servo = servo; }

private:
    sina_class* _sina = nullptr;
    GTD35Adapter* _gtd35 = nullptr;
    ServoAdapter* _servo = nullptr;
};

// ============================================================================
// GLOBAL INSTANCES & INIT
// ============================================================================

// Global component instances (4 components only)
extern TDAxis12Adapter tdaAxis12;
extern GTD35Adapter gtd35Adapter;
extern ServoAdapter servoAdapter;
extern SpinBackMotor spinBackMotor;

// New class instances
extern amirreza_class amirreza;
extern sina_class sina;
extern sadra_class sadra;
extern king_forati_class king;
extern sina_rotation_class sina_rotation;

// Initialize all components
// For GTD35 mode (UART):
void saratechBeginGTD35(TwoWire* i2cWire = &Wire, uint8_t tdaAddress = 0x10,
                        HardwareSerial& serialPort = Serial1,
                        int8_t directionPin = -1, uint8_t txLevel = HIGH,
                        uint32_t baud = 115200) {
    // Initialize TDAxis12 (I2C)
    tdaAxis12.begin(i2cWire, tdaAddress);

    // Initialize GTD35 (UART)
    gtd35Adapter.begin(serialPort, baud, directionPin, txLevel);

    // Initialize Servo
    servoAdapter.begin();

    // Initialize Spin Back Motor
    spinBackMotor.begin();

    // Initialize new classes
    amirreza.begin();
    sina.begin(&gtd35Adapter, &spinBackMotor);
    sadra.begin(&sina);
    king.begin(&sina);
    sina_rotation.begin(&sina, &servoAdapter);

    // Amirreza setup shooter (PC13 command, PA4 feedback)
    // This mirrors original initialization
    // amirreza.shoot(PC13, PA4);  // Uncomment if needed

    // Read distance sensor baselines (average 10 samples)
    int kf_sum = 0, kr_sum = 0, kb_sum = 0, kl_sum = 0;
    for (int i = 0; i < 10; i++) {
        kf_sum += analogRead(PA5);
        kr_sum += analogRead(PA6);
        kb_sum += analogRead(PA7);
        kl_sum += analogRead(PB0);
        delay(5);
    }
    RobotState::instance().setBaselines(kf_sum / 10, kr_sum / 10, kb_sum / 10, kl_sum / 10);

    // Initialize TSOP channel selector pins
    pinMode(PA8, OUTPUT);
    pinMode(PB1, OUTPUT);
    pinMode(PC14, OUTPUT);
    pinMode(PC15, OUTPUT);
}

// Update all sensors (call in main loop)
void saratechUpdateSensors() {
    RobotState& state = RobotState::instance();

    // Read TDAxis12 yaw
    float yaw = tdaAxis12.read();

    // Read distance sensors
    if (state.baselinesSet) {
        int rawFront = analogRead(PA5);
        int rawRight = analogRead(PA6);
        int rawBack = analogRead(PA7);
        int rawLeft = analogRead(PB0);

        float distFront = state.adcToCm(rawFront, state.baselineFront);
        float distRight = state.adcToCm(rawRight, state.baselineRight);
        float distBack = state.adcToCm(rawBack, state.baselineBack);
        float distLeft = state.adcToCm(rawLeft, state.baselineLeft);

        state.updateDistances(distFront, distRight, distBack, distLeft,
                              distFront > 0 && distFront < 300,
                              distRight > 0 && distRight < 300,
                              distBack > 0 && distBack < 300,
                              distLeft > 0 && distLeft < 300);
    }

    // Read TSOP ball sensor
    float ballAngle, ballDistance;
    if (tdaAxis12.getBallData(ballAngle, ballDistance)) {
        bool ballValid = ballDistance > 2000.0f;
        bool inKicker = false;
        if (digitalRead(PA4) == LOW && ballDistance > 7500.0f &&
            ballAngle > -90.0f && ballAngle < 90.0f) {
            inKicker = true;
        }
        state.updateBall(ballAngle, ballDistance, ballValid, inKicker);
    }

    // Read side sensors
    int shr = analogRead(PA2);
    int shl = analogRead(PA3);
    int shb = analogRead(PA1);
    state.updateSideSensors(shr, shl, shb);

    // Update position using Sadra's mapcalc
    sadra.mapcalc(state.distBack, state.distFront, state.distLeft, state.distRight);
}

// Helper functions
namespace saratech {
    inline float normalizeAngle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }
    inline float rad(float deg) { return deg * PI / 180.0f; }
    inline float deg(float rad) { return rad * 180.0f / PI; }
    template<typename T>
    inline T clamp(T val, T minVal, T maxVal) {
        return val < minVal ? minVal : (val > maxVal ? maxVal : val);
    }
}

#endif // SARATECH_H
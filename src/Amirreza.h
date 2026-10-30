#ifndef SARATECH_AMIRREZA_H
#define SARATECH_AMIRREZA_H

#include <Arduino.h>
#include "RobotState.h"
#include "Sina.h"
#include "ServoAdapter.h"

// Amirreza - Ball-facing rotation, shooting, and servo aiming
// Provides:
// - clapm(maxRotation, robotX, robotY, ballX, ballY) - rotate toward ball
// - shoot(commandPin, feedbackPin) - initialize shooter
// - clapmshoot(robotX, robotY, goalX, goalY) - align to goal and shoot
// - shootclamp() - servo aiming with fixed robot yaw
// - smshoot(speed, n, i, z) - smart combined shooting

class Amirreza {
public:
    Amirreza() : _sina(nullptr), _servo(nullptr),
                 _commandPin(PC13), _feedbackPin(PA4),
                 _shooterState(SHOOTER_IDLE), _shootStart(0),
                 _smshootState(SMS_IDLE), _smshootStart(0),
                 _alignStart(0) {}

    void begin(Sina* sina = nullptr, ServoAdapter* servo = nullptr) {
        _sina = sina;
        _servo = servo;
    }

    // Initialize shooter with command and feedback pins
    // commandPin: output pin (PC13, active HIGH to fire)
    // feedbackPin: input pin (PA4, active LOW = ready/loaded)
    void shoot(uint8_t commandPin, uint8_t feedbackPin) {
        _commandPin = commandPin;
        _feedbackPin = feedbackPin;

        pinMode(_commandPin, OUTPUT);
        pinMode(_feedbackPin, INPUT_PULLUP);  // Active LOW input

        digitalWrite(_commandPin, LOW);  // Ensure safe state
        RobotState::instance().shooterReady = checkBallReady();
        _shooterState = SHOOTER_IDLE;
    }

    // Rotate robot toward ball using yaw feedback
    // maxRotation: max yaw rotation allowed (degrees)
    // robotX, robotY: robot position (cm, field coords)
    // ballX, ballY: ball position (cm, field coords)
    void clapm(float maxRotation, float robotX, float robotY,
               float ballX, float ballY) {
        RobotState& state = RobotState::instance();

        if (!state.yawValid || state.isYawStale()) {
            state.setError(RobotState::ERR_YAW_STALE);
            if (_sina) _sina->stop();
            return;
        }

        // Calculate direction to ball
        float dx = ballX - robotX;
        float dy = ballY - robotY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 1.0f) {
            // Already at ball position
            if (_sina) _sina->stop();
            return;
        }

        // Target angle (field coordinates, 0=forward/+Y, 90=right/+X)
        float targetAngle = degrees(atan2(dx, dy));

        // Normalize angles
        normalizeAngle(targetAngle);
        float currentYaw = state.yaw;
        normalizeAngle(currentYaw);

        // Calculate error
        float error = targetAngle - currentYaw;
        // Normalize to -180..180
        if (error > 180) error -= 360;
        if (error < -180) error += 360;

        // Apply maxRotation limit
        if (fabs(error) > maxRotation) {
            error = (error > 0) ? maxRotation : -maxRotation;
        }

        if (fabs(error) < 2.0f) {
            // Within alignment tolerance - stop
            if (_sina) _sina->stop();
        } else {
            // Rotate toward target
            // For in-place rotation: all wheels same direction
            int16_t rotSpeed = (int16_t)(state.config.maxMotorCmd * 0.3);
            if (error < 0) rotSpeed = -rotSpeed;  // Rotate counterclockwise (left)

            if (_sina) {
                _sina->stop();  // Stop translation first
                if (_sina->getMotorDriver()) {
                    _sina->getMotorDriver()->setMotorRaw(rotSpeed, rotSpeed, rotSpeed, rotSpeed);
                }
            }
        }
    }

    // Align with goal and shoot
    // robotX, robotY: robot position
    // goalX, goalY: goal position
    void clapmshoot(float robotX, float robotY, float goalX, float goalY) {
        RobotState& state = RobotState::instance();

        // Check if already shooting (prevent duplicate shots)
        if (_shooterState == SHOOTER_FIRING) {
            updateShooterState();
            return;
        }

        if (!state.yawValid || state.isYawStale()) {
            state.setError(RobotState::ERR_YAW_STALE);
            return;
        }

        // Calculate direction to goal
        float dx = goalX - robotX;
        float dy = goalY - robotY;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 1.0f) return;

        // Target angle (0° = forward, so use atan2(dx, dy))
        float targetAngle = degrees(atan2(dx, dy));
        normalizeAngle(targetAngle);

        float currentYaw = state.yaw;
        normalizeAngle(currentYaw);

        float error = targetAngle - currentYaw;
        if (error > 180) error -= 360;
        if (error < -180) error += 360;

        if (fabs(error) > state.config.shootAlignmentTol) {
            // Need more alignment
            _alignStart = millis();
            // Rotate toward goal
            float rotSpeed = 0.3f;
            int16_t motorCmd = (int16_t)(state.config.maxMotorCmd * rotSpeed);
            if (error < 0) motorCmd = -motorCmd;

            if (_sina) {
                _sina->stop();
                if (_sina->getMotorDriver()) {
                    _sina->getMotorDriver()->setMotorRaw(motorCmd, motorCmd, motorCmd, motorCmd);
                }
            }
            return;
        }

        // Within alignment tolerance - check if ready to shoot
        if (!_readyToFire()) {
            return;
        }

        // Execute shot
        fireShooter();
    }

    // Servo aiming with fixed robot yaw
    void shootclamp() {
        RobotState& state = RobotState::instance();

        // Check if already firing
        if (_shooterState == SHOOTER_FIRING) {
            updateShooterState();
            return;
        }

        // Target direction is calculated from current state
        // Use goal direction from last clapmshoot or stored target
        float targetDir = 0.0f;

        // Get target from state's stored target coordinates
        if (state.hasTarget) {
            float dx = state.targetX - state.posX;
            float dy = state.targetY - state.posY;
            targetDir = degrees(atan2(dx, dy));
        }

        // Calculate relative angle (servo angle)
        // Servo 90° = robot forward (0° yaw)
        // Servo angle = 90 + angle from forward
        float servoAngle = 90.0f + targetDir;  // Simple model: servo follows target direction

        // Clamp to usable range
        if (servoAngle < state.config.servoMin) servoAngle = state.config.servoMin;
        if (servoAngle > state.config.servoMax) servoAngle = state.config.servoMax;

        // Set servo angle
        if (_servo && _servo->isAttached()) {
            _servo->setAngle(servoAngle);
        }

        // Hold yaw (robot stationary)
        if (fabs(targetDir) < 5.0f) {
            // Nearly aligned with target - ready to shoot
            if (!_readyToFire()) return;
            fireShooter();
        }
    }

    // Combined smart shooting
    // speed: approach speed cm/s
    // n: minimum distance from goal cm
    // i: max robot body yaw rotation degrees
    // z: max servo rotation relative to center degrees
    void smshoot(int speed, int n, int i, int z) {
        RobotState& state = RobotState::instance();

        switch (_smshootState) {
            case SMS_IDLE:
                // Reset state
                _smshootState = SMS_APPROACH;
                _smshootStart = millis();
                // Fall through

            case SMS_APPROACH:
                // Move toward opponent goal until distance n
                {
                    float goalX = state.posX;
                    float goalY = state.config.fieldLength;  // Opponent goal at Y=240
                    float dx = goalX - state.posX;
                    float dy = goalY - state.posY;
                    float distToGoal = sqrt(dx * dx + dy * dy);

                    if (distToGoal <= n) {
                        _smshootState = SMS_ALIGN_YAW;
                    } else if (distToGoal > 5.0f) {
                        // Approach goal
                        float dirX = dx / distToGoal;
                        float dirY = dy / distToGoal;

                        // Reduce speed as we approach
                        float approachSpeed = min((float)speed, state.config.maxSpeedCmS);

                        if (_sina) {
                            _sina->motor(approachSpeed, dirX * 100, dirY * 100, 200, 0);
                        }
                    }
                }
                break;

            case SMS_ALIGN_YAW:
                // Rotate robot toward goal, limit by i degrees
                {
                    float goalY = state.config.fieldLength;
                    float dx = 0;  // Directly forward
                    float dy = goalY - state.posY;
                    float targetAngle = degrees(atan2(dx, dy));  // Should be 0 (forward)
                    normalizeAngle(targetAngle);

                    float currentYaw = state.yaw;
                    normalizeAngle(currentYaw);

                    float error = targetAngle - currentYaw;
                    if (error > 180) error -= 360;
                    if (error < -180) error += 360;

                    if (fabs(error) <= i) {
                        // Within allowed rotation - aligned
                        _smshootState = SMS_CHECK_ALIGNMENT;
                    } else {
                        // Rotate toward goal, limited by i
                        _sina->stop();
                        float rotSpeed = 0.3f;
                        int16_t motorCmd = (int16_t)(state.config.maxMotorCmd * rotSpeed);
                        if (error < 0) motorCmd = -motorCmd;
                        if (_sina && _sina->getMotorDriver()) {
                            _sina->getMotorDriver()->setMotorRaw(motorCmd, motorCmd, motorCmd, motorCmd);
                        }
                        _smshootState = SMS_SERVO_ALIGN;
                    }
                }
                break;

            case SMS_CHECK_ALIGNMENT:
                // Check if ball is detected and ready
                if (_readyToFire()) {
                    _smshootState = SMS_FIRE;
                } else {
                    // Use servo to compensate
                    _smshootState = SMS_SERVO_ALIGN;
                }
                break;

            case SMS_SERVO_ALIGN:
                // Servo aim within z degrees of center
                {
                    float goalY = state.config.fieldLength;
                    float dx = state.posX;
                    float dy = goalY - state.posY;
                    float targetDir = degrees(atan2(dx, dy));
                    normalizeAngle(targetDir);

                    float servoAngle = state.config.servoCenter + targetDir;

                    // Apply z limit (relative to center)
                    if (servoAngle > state.config.servoCenter + z)
                        servoAngle = state.config.servoCenter + z;
                    if (servoAngle < state.config.servoCenter - z)
                        servoAngle = state.config.servoCenter - z;

                    // Clamp to mechanical limits
                    if (servoAngle < state.config.servoMin) servoAngle = state.config.servoMin;
                    if (servoAngle > state.config.servoMax) servoAngle = state.config.servoMax;

                    if (_servo && _servo->isAttached()) {
                        _servo->setAngle(servoAngle);
                    }

                    if (_readyToFire()) {
                        _smshootState = SMS_FIRE;
                    }
                }
                break;

            case SMS_FIRE:
                fireShooter();
                break;

            case SMS_DONE:
            case SMS_TIMEOUT:
                // Wait for reset
                break;
        }

        // Check timeout
        if (millis() - _smshootStart > 10000) {  // 10 second timeout
            _smshootState = SMS_TIMEOUT;
            state.setError(RobotState::ERR_MOVEMENT_TIMEOUT);
            if (_sina) _sina->stop();
        }

        // Update shooter state
        if (_shooterState == SHOOTER_FIRING) {
            updateShooterState();
        }
    }

    // Reset smshoot sequence (call when ready for another shot)
    void resetSmshoot() {
        _smshootState = SMS_IDLE;
    }

    // Check if currently firing
    bool isFiring() const {
        return _shooterState == SHOOTER_FIRING;
    }

    // Check ball ready (PA4 active LOW)
    bool checkBallReady() {
        if (digitalRead(_feedbackPin) == LOW) {
            RobotState::instance().ballInKicker = true;
            return true;
        }
        RobotState::instance().ballInKicker = false;
        return false;
    }

    // Get shooter state string (for debugging)
    const char* getShooterStateStr() const {
        switch (_shooterState) {
            case SHOOTER_IDLE: return "IDLE";
            case SHOOTER_READY: return "READY";
            case SHOOTER_FIRING: return "FIRING";
            case SHOOTER_PULSE_COMPLETE: return "PULSE_COMPLETE";
            case SHOOTER_NOT_READY: return "NOT_READY";
            default: return "UNKNOWN";
        }
    }

    const char* getSmshootStateStr() const {
        switch (_smshootState) {
            case SMS_IDLE: return "IDLE";
            case SMS_APPROACH: return "APPROACH";
            case SMS_ALIGN_YAW: return "ALIGN_YAW";
            case SMS_CHECK_ALIGNMENT: return "CHECK_ALIGNMENT";
            case SMS_SERVO_ALIGN: return "SERVO_ALIGN";
            case SMS_FIRE: return "FIRE";
            case SMS_DONE: return "DONE";
            case SMS_TIMEOUT: return "TIMEOUT";
            default: return "UNKNOWN";
        }
    }

private:
    Sina* _sina;
    ServoAdapter* _servo;
    uint8_t _commandPin;
    uint8_t _feedbackPin;

    enum ShooterState {
        SHOOTER_IDLE,
        SHOOTER_READY,
        SHOOTER_FIRING,
        SHOOTER_PULSE_COMPLETE,
        SHOOTER_NOT_READY
    };

    ShooterState _shooterState;
    uint32_t _shootStart;

    enum SmshootState {
        SMS_IDLE,
        SMS_APPROACH,
        SMS_ALIGN_YAW,
        SMS_CHECK_ALIGNMENT,
        SMS_SERVO_ALIGN,
        SMS_FIRE,
        SMS_DONE,
        SMS_TIMEOUT
    };

    SmshootState _smshootState;
    uint32_t _smshootStart;
    uint32_t _alignStart;

    bool _readyToFire() {
        return checkBallReady();
    }

    void fireShooter() {
        RobotState& state = RobotState::instance();
        state.shooterFiring = true;
        _shooterState = SHOOTER_FIRING;
        _shootStart = millis();

        // Fire for minimum 200ms
        digitalWrite(_commandPin, HIGH);
    }

    void updateShooterState() {
        RobotState& state = RobotState::instance();

        if (_shooterState == SHOOTER_FIRING) {
            if (millis() - _shootStart >= RobotState::instance().SHOOTER_PULSE_MS) {
                // End pulse
                digitalWrite(_commandPin, LOW);
                state.shooterFiring = false;
                _shooterState = SHOOTER_PULSE_COMPLETE;

                // Reset to idle after brief cooldown
                static uint32_t cooldownStart = 0;
                if (cooldownStart == 0) cooldownStart = millis();
                if (millis() - cooldownStart > 500) {
                    _shooterState = SHOOTER_IDLE;
                    cooldownStart = 0;
                }
            }
        }
    }

    void normalizeAngle(float& angle) {
        while (angle > 180) angle -= 360;
        while (angle < -180) angle += 360;
    }
};

#endif // SARATECH_AMIRREZA_H
#ifndef SARATECH_SADRA_H
#define SARATECH_SADRA_H

#include <Arduino.h>
#include "RobotState.h"
#include "Sina.h"

// Sadra - Field mapping and coordinate-based movement
// Provides sadra.mapcalc() and sadra.move()

class Sadra {
public:
    Sadra() : _moveState(IDLE), _moveStartTime(0) {}

    void begin(Sina* sina = nullptr) {
        _sina = sina;
    }

    // Process four distance measurements and update position estimate
    // Inputs: calibrated distances in cm (or raw ADC if baselines not set)
    // Directions: rear, front, left, right (matches original sensor order)
    void mapcalc(float rearDistance, float frontDistance,
                 float leftDistance, float rightDistance) {
        RobotState& state = RobotState::instance();

        // Validate inputs (basic sanity check)
        bool vRear = rearDistance >= 0 && rearDistance < 300;
        bool vFront = frontDistance >= 0 && frontDistance < 300;
        bool vLeft = leftDistance >= 0 && leftDistance < 300;
        bool vRight = rightDistance >= 0 && rightDistance < 300;

        // Store distances
        state.updateDistances(frontDistance, rightDistance,
                              rearDistance, leftDistance,
                              vFront, vRight, vRear, vLeft);

        // Update position estimate using available sensors
        updatePosition();
    }

    // Move to field coordinates (x, y in cm) with max speed
    // Non-blocking - call repeatedly in loop
    // Returns true when target reached
    bool move(int targetX, int targetY, int speed) {
        RobotState& state = RobotState::instance();

        if (!state.posValid || state.isPosStale()) {
            state.setError(RobotState::ERR_POS_INVALID);
            return false;
        }

        // Store target
        state.targetX = targetX;
        state.targetY = targetY;
        state.hasTarget = true;

        // Calculate displacement
        float dx = targetX - state.posX;
        float dy = targetY - state.posY;
        float distance = sqrt(dx * dx + dy * dy);

        // Check arrival
        if (distance <= state.config.arrivalTolerance) {
            if (_sina) _sina->stop();
            state.hasTarget = false;
            return true;
        }

        // Calculate direction
        float dirX = dx / distance;
        float dirY = dy / distance;

        // Speed reduction as we approach (proportional)
        float approachSpeed = speed;
        // Optional: reduce speed when close
        if (distance < 30.0f) {
            approachSpeed = max(5.0f, speed * (distance / 30.0f));
        }

        // Use Sina for movement
        if (_sina) {
            _sina->motor(approachSpeed, dirX * 100, dirY * 100, 200, 0);
        }

        _moveState = MOVING;
        _moveStartTime = millis();
        return false;
    }

    // Check if currently moving to target
    bool isMoving() const {
        return _moveState == MOVING && RobotState::instance().hasTarget;
    }

    // Stop current movement
    void stop() {
        _moveState = IDLE;
        RobotState::instance().hasTarget = false;
        if (_sina) _sina->stop();
    }

    // Get distance to target
    float distanceToTarget() const {
        RobotState& state = RobotState::instance();
        if (!state.hasTarget) return 0;
        float dx = state.targetX - state.posX;
        float dy = state.targetY - state.posY;
        return sqrt(dx * dx + dy * dy);
    }

    enum MoveState { IDLE, MOVING };

private:
    Sina* _sina = nullptr;
    MoveState _moveState = IDLE;
    uint32_t _moveStartTime = 0;

    // Update position from distance sensors
    void updatePosition() {
        RobotState& state = RobotState::instance();

        // Get current yaw (robot orientation)
        float yaw = state.yaw;
        if (!state.yawValid || state.isYawStale()) {
            // Can't transform sensor readings without valid yaw
            state.setError(RobotState::ERR_YAW_STALE);
            return;
        }

        // Convert yaw to radians
        float yawRad = radians(yaw);

        // Sensor directions in robot frame:
        // Front = +Y, Right = +X, Back = -Y, Left = -X
        // Transform to field frame using yaw
        // Field: +X = right, +Y = forward

        // For each valid sensor, compute wall intersection
        // This is a simplified approach - real implementation depends on
        // which walls are visible

        float estX = state.posX;
        float estY = state.posY;
        int validCount = 0;

        // If we can see front and back walls (Y boundaries)
        if (state.distValid[0] && state.distValid[2]) {  // front and back
            // Front sensor sees wall at Y = fieldLength
            // Back sensor sees wall at Y = 0
            float yFromFront = state.config.fieldLength - state.distFront;
            float yFromBack = state.distBack;
            // Average if both valid
            estY = (yFromFront + yFromBack) / 2.0f;
            validCount++;
        } else if (state.distValid[0]) {  // front only
            estY = state.config.fieldLength - state.distFront;
            validCount++;
        } else if (state.distValid[2]) {  // back only
            estY = state.distBack;
            validCount++;
        }

        // If we can see left and right walls (X boundaries)
        if (state.distValid[3] && state.distValid[1]) {  // left and right
            float xFromRight = state.config.fieldWidth - state.distRight;
            float xFromLeft = state.distLeft;
            estX = (xFromRight + xFromLeft) / 2.0f;
            validCount++;
        } else if (state.distValid[3]) {  // left only
            estX = state.distLeft;
            validCount++;
        } else if (state.distValid[1]) {  // right only
            estX = state.config.fieldWidth - state.distRight;
            validCount++;
        }

        // If we have at least one valid estimate, update position
        if (validCount > 0) {
            // Clamp to field bounds
            estX = constrain(estX, 0, state.config.fieldWidth);
            estY = constrain(estY, 0, state.config.fieldLength);

            state.updatePosition(estX, estY, true);
            state.clearError(RobotState::ERR_POS_INVALID);
        } else {
            state.setError(RobotState::ERR_POS_INVALID);
        }
    }
};

#endif // SARATECH_SADRA_H
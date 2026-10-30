#ifndef SARATECH_KING_H
#define SARATECH_KING_H

#include <Arduino.h>
#include "RobotState.h"
#include "Sina.h"

// King - Return to start position
// Provides king.forati(speed, xtarg, ytarg)

class King {
public:
    King() : _sina(nullptr), _state(IDLE), _startTime(0) {}

    void begin(Sina* sina = nullptr) {
        _sina = sina;
    }

    // Return to specified field coordinates
    // speed: maximum speed cm/s
    // xtarg, ytarg: target position in cm (field coordinates)
    void forati(int speed, int xtarg, int ytarg) {
        RobotState& state = RobotState::instance();

        if (!state.posValid || state.isPosStale()) {
            state.setError(RobotState::ERR_POS_INVALID);
            if (_sina) _sina->stop();
            return;
        }

        // Calculate displacement
        float dx = xtarg - state.posX;
        float dy = ytarg - state.posY;
        float distance = sqrt(dx * dx + dy * dy);

        // Check arrival
        if (distance <= state.config.arrivalTolerance) {
            if (_sina) _sina->stop();
            _state = IDLE;
            return;
        }

        _state = RETURNING;

        // Direction
        float dirX = dx / distance;
        float dirY = dy / distance;

        // Distance-proportional speed model
        // v_cmd = min(v_max, K_d * d)
        // Use K_d = speed / 50.0 for reasonable deceleration
        float kd = (float)speed / 50.0f;
        float targetSpeed = min((float)speed, kd * distance);
        targetSpeed = max(targetSpeed, 5.0f);  // Minimum speed

        // Use Sina for movement
        if (_sina) {
            _sina->motor(targetSpeed, dirX * 100, dirY * 100, 200, 0);
        }
    }

    // Check if currently returning
    bool isReturning() const {
        return _state == RETURNING;
    }

    // Check if at target
    bool atTarget() const {
        return _state == IDLE;
    }

    // Stop return movement
    void stop() {
        _state = IDLE;
        if (_sina) _sina->stop();
    }

    // Get distance to target
    float distanceToTarget(int xtarg, int ytarg) const {
        RobotState& state = RobotState::instance();
        if (!state.posValid) return -1;
        float dx = xtarg - state.posX;
        float dy = ytarg - state.posY;
        return sqrt(dx * dx + dy * dy);
    }

private:
    Sina* _sina;
    enum State { IDLE, RETURNING };
    State _state;
    uint32_t _startTime;
};

#endif // SARATECH_KING_H
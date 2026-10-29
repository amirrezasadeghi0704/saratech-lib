/*
 * SARATECH - Full Match Example
 *
 * Complete robot behavior for a football match:
 * - Field mapping and positioning
 * - Ball detection and tracking
 * - Goal alignment and shooting
 * - Return to start position
 */

#include <saratech.h>

TwoWire i2c2(2);

enum MatchState {
    STARTUP,
    FIND_BALL,
    APPROACH_BALL,
    ALIGN_GOAL,
    SHOOT,
    RETURN_START,
    IDLE
};

MatchState matchState = STARTUP;
uint32_t stateStartTime = 0;
bool kickoff = true;

void setup() {
    Serial.begin(115200);
    delay(1000);

    saratechBegin(&i2c2, 0x10);

    // Set initial position (our goal line center)
    RobotState::instance().updatePosition(90.0f, 10.0f, true);

    Serial.println("SARATECH Full Match Example");
    Serial.println("Starting match...");
}

void loop() {
    saratechUpdateSensors();

    RobotState& state = RobotState::instance();

    // Print status every 500ms
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 500) {
        Serial.print("State: ");
        switch (matchState) {
            case STARTUP: Serial.print("STARTUP"); break;
            case FIND_BALL: Serial.print("FIND_BALL"); break;
            case APPROACH_BALL: Serial.print("APPROACH_BALL"); break;
            case ALIGN_GOAL: Serial.print("ALIGN_GOAL"); break;
            case SHOOT: Serial.print("SHOOT"); break;
            case RETURN_START: Serial.print("RETURN_START"); break;
            case IDLE: Serial.print("IDLE"); break;
        }
        Serial.print(" Pos: (");
        Serial.print(state.posX, 1);
        Serial.print(", ");
        Serial.print(state.posY, 1);
        Serial.print(") Yaw: ");
        Serial.print(state.yaw, 1);
        Serial.print(" Ball: ");
        Serial.print(state.ballValid ? "YES" : "NO");
        if (state.ballValid) {
            Serial.print(" angle=");
            Serial.print(state.ballAngle, 1);
        }
        Serial.println();
        lastPrint = millis();
    }

    switch (matchState) {
        case STARTUP:
            // Initial positioning
            if (kickoff) {
                sadra.move(90, 60, 15);  // Move to midfield
                if (sadra.distanceToTarget() < 10) {
                    matchState = FIND_BALL;
                    kickoff = false;
                }
            } else {
                matchState = FIND_BALL;
            }
            break;

        case FIND_BALL:
            // Search for ball (spin or move to center)
            if (state.ballValid) {
                matchState = APPROACH_BALL;
                stateStartTime = millis();
            } else {
                // Slow search pattern
                sina.motor(5, 90, 0, 200, 0);  // Slow strafe right
            }
            break;

        case APPROACH_BALL: {
            if (!state.ballValid) {
                matchState = FIND_BALL;
                break;
            }

            // Estimate ball position
            float ballX = state.posX + state.ballDistance * 0.01 * sin(radians(state.ballAngle));
            float ballY = state.posY + state.ballDistance * 0.01 * cos(radians(state.ballAngle));

            // Move toward ball
            float dx = ballX - state.posX;
            float dy = ballY - state.posY;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist < 20) {  // Close enough to ball
                matchState = ALIGN_GOAL;
            } else {
                float dirX = dx / dist;
                float dirY = dy / dist;
                sina.motor(15, dirX * 100, dirY * 100, 200, 0);
            }
            break;
        }

        case ALIGN_GOAL:
            if (!state.ballInKicker) {
                matchState = APPROACH_BALL;
                break;
            }

            // Align to opponent goal
            amirreza.clapmshoot(state.posX, state.posY, 90, 240);

            if (strcmp(amirreza.getShooterStateStr(), "PULSE_COMPLETE") == 0) {
                matchState = RETURN_START;
            }
            break;

        case SHOOT:
            // Handled by clapmshoot
            break;

        case RETURN_START:
            // Return to our goal area
            king.forati(15, 90, 10);
            if (king.distanceToTarget(90, 10) < 5) {
                matchState = IDLE;
            }
            break;

        case IDLE:
            sina.stop();
            break;
    }

    delay(20);
}
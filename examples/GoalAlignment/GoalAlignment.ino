/*
 * SARATECH - Goal Alignment Example
 *
 * Demonstrates amirreza.clapm() for ball-facing rotation
 * and amirreza.clapmshoot() for goal alignment.
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);
    delay(1000);

    saratechBegin(&i2c2, 0x10);

    Serial.println("SARATECH Goal Alignment Example");
}

void loop() {
    saratechUpdateSensors();

    RobotState& state = RobotState::instance();

    // Print yaw and ball info
    Serial.print("Yaw: ");
    Serial.print(state.yaw, 1);
    Serial.print(" Ball Angle: ");
    Serial.print(state.ballAngle, 1);
    Serial.print(" Ball Valid: ");
    Serial.println(state.ballValid);

    if (state.ballValid) {
        // Rotate to face ball
        // maxRotation = 45 degrees limit
        // Robot position from state
        // Ball position estimated from ball angle/distance
        float ballX = state.posX + state.ballDistance * 0.01 * sin(radians(state.ballAngle));
        float ballY = state.posY + state.ballDistance * 0.01 * cos(radians(state.ballAngle));

        amirreza.clapm(45, state.posX, state.posY, ballX, ballY);

        Serial.print("Rotating to ball... Target: (");
        Serial.print(ballX, 1);
        Serial.print(", ");
        Serial.print(ballY, 1);
        Serial.println(")");
    } else {
        // No ball - align to opponent goal (Y=240)
        amirreza.clapmshoot(state.posX, state.posY, 90, 240);
        Serial.println("No ball - aligning to goal");
    }

    delay(100);
}
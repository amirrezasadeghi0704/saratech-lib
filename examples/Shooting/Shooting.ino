/*
 * SARATECH - Shooting Example
 *
 * Demonstrates amirreza shooting functions:
 * - shoot() - initialize shooter
 * - clapmshoot() - align to goal and shoot
 * - shootclamp() - servo aim with fixed yaw
 * - smshoot() - smart combined shooting
 */

#include <saratech.h>

TwoWire i2c2(2);

enum DemoState { WAIT_BALL, ALIGN_GOAL, SERVO_AIM, SMART_SHOOT };
DemoState demoState = WAIT_BALL;
uint32_t lastStateChange = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    saratechBegin(&i2c2, 0x10);

    Serial.println("SARATECH Shooting Example");
    Serial.println("Waiting for ball detection...");
}

void loop() {
    saratechUpdateSensors();

    RobotState& state = RobotState::instance();

    // Print ball status
    Serial.print("Ball: angle=");
    Serial.print(state.ballAngle, 1);
    Serial.print(" dist=");
    Serial.print(state.ballDistance, 0);
    Serial.print(" valid=");
    Serial.print(state.ballValid);
    Serial.print(" inKicker=");
    Serial.println(state.ballInKicker);

    // State machine
    switch (demoState) {
        case WAIT_BALL:
            if (state.ballValid && state.ballInKicker) {
                demoState = ALIGN_GOAL;
                lastStateChange = millis();
                Serial.println("Ball detected in kicker! Aligning to goal...");
            }
            break;

        case ALIGN_GOAL:
            // Opponent goal at Y=240, center X=90
            amirreza.clapmshoot(state.posX, state.posY, 90, 240);

            if (amirreza.getShooterStateStr()[0] == 'P') {  // PULSE_COMPLETE
                demoState = SERVO_AIM;
                lastStateChange = millis();
                Serial.println("Shot fired! Now servo aim demo...");
            }
            break;

        case SERVO_AIM:
            // Keep yaw fixed, aim with servo
            amirreza.shootclamp();

            if (amirreza.getShooterStateStr()[0] == 'P') {
                demoState = SMART_SHOOT;
                lastStateChange = millis();
                Serial.println("Servo shot fired! Now smart shoot demo...");
            }
            break;

        case SMART_SHOOT:
            // Smart shooting: approach goal, align, shoot
            // speed=15 cm/s, n=30cm from goal, i=30deg robot rot, z=20deg servo
            amirreza.smshoot(15, 30, 30, 20);

            if (amirreza.getSmshootStateStr()[0] == 'D' ||  // DONE
                amirreza.getSmshootStateStr()[0] == 'T') {  // TIMEOUT
                amirreza.resetSmshoot();
                demoState = WAIT_BALL;
                lastStateChange = millis();
                Serial.println("Smart shoot complete. Waiting for ball...");
            }
            break;
    }

    delay(50);
}
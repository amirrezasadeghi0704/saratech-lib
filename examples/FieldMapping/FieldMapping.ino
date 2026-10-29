/*
 * SARATECH - Field Mapping Example
 *
 * Demonstrates sadra.mapcalc() for position estimation
 * and sadra.move() for coordinate-based movement.
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);
    delay(1000);

    saratechBegin(&i2c2, 0x10);

    Serial.println("SARATECH Field Mapping Example");
}

void loop() {
    // Update all sensors
    saratechUpdateSensors();

    RobotState& state = RobotState::instance();

    // Print position estimate
    Serial.print("Pos: (");
    Serial.print(state.posX, 1);
    Serial.print(", ");
    Serial.print(state.posY, 1);
    Serial.print(") Valid: ");
    Serial.print(state.posValid);
    Serial.print(" Yaw: ");
    Serial.print(state.yaw, 1);
    Serial.print(" Valid: ");
    Serial.println(state.yawValid);

    // Print distances
    Serial.print("Dist - F:");
    Serial.print(state.distFront, 1);
    Serial.print(" R:");
    Serial.print(state.distRight, 1);
    Serial.print(" B:");
    Serial.print(state.distBack, 1);
    Serial.print(" L:");
    Serial.println(state.distLeft, 1);

    // Move to center of field (90, 120)
    static bool targetSet = false;
    if (!targetSet && state.posValid) {
        sadra.move(90, 120, 15);
        targetSet = true;
    }

    if (sadra.isMoving()) {
        Serial.print("Moving to target... Dist: ");
        Serial.println(sadra.distanceToTarget(), 1);
    }

    delay(100);
}
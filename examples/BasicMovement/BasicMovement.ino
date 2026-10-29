/*
 * SARATECH - Basic Movement Example
 *
 * Demonstrates sina.motor() for omnidirectional movement.
 *
 * Hardware: STM32 Blue Pill with 4 GTD35 motors (PWM mode)
 *
 * Connections:
 * - ML1: Dir PB15, PWM PB9
 * - ML2: Dir PB14, PWM PB8
 * - MR2: Dir PB13, PWM PB7
 * - MR1: Dir PB12, PWM PB6
 * - TDAxis12: I2C2 (PB10=SCL, PB11=SDA), address 0x10
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize SARATECH with custom I2C bus
    saratechBegin(&i2c2, 0x10);

    Serial.println("SARATECH Basic Movement Example");
    Serial.println("Moving right at 10 cm/s for 3 seconds...");
}

void loop() {
    // Update sensors
    saratechUpdateSensors();

    // Move right at 10 cm/s
    // sina.motor(speed, x, y, tdaxisGain, acceleration)
    // x=90, y=0 -> direction vector pointing right
    sina.motor(10, 90, 0, 200, 0);

    delay(3000);

    // Stop
    sina.stop();

    Serial.println("Stopped. Waiting 2 seconds...");
    delay(2000);

    // Move forward at 15 cm/s
    Serial.println("Moving forward at 15 cm/s...");
    sina.motor(15, 0, 90, 200, 0);
    delay(3000);
    sina.stop();

    // Move diagonally (forward-right)
    Serial.println("Moving diagonally at 10 cm/s...");
    sina.motor(10, 90, 90, 200, 0);
    delay(3000);
    sina.stop();

    // Test zero vector (should stop)
    Serial.println("Zero vector test...");
    sina.motor(10, 0, 0, 200, 0);
    delay(1000);
    sina.stop();

    Serial.println("Test complete. Restarting in 5 seconds...");
    delay(5000);
}
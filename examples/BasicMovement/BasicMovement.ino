/**
 * @file BasicMovement.ino
 * @brief Basic movement with amirreza_class
 * @version 2.0.0
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
    amirreza.setTdaxisGain(200);
}

void loop() {
    saratechUpdateSensors();

    // Move forward at 10 cm/s with 5 cm/s^2 acceleration
    amirreza.move(10, 0, 100, 5);
    delay(2000);

    // Move right at 15 cm/s
    amirreza.move(15, 90, 0, 5);
    delay(2000);

    // Move backward at 10 cm/s
    amirreza.move(10, 0, -100, 5);
    delay(2000);

    // Move left at 15 cm/s
    amirreza.move(15, -90, 0, 5);
    delay(2000);

    // Stop
    amirreza.stop();
    delay(1000);
}
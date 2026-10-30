/**
 * @file RotationServo.ino
 * @brief Rotation with sina_class + servo
 * @version 2.0.0
 */

#include <saratech.h>

void setup() {
    saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
    amirreza.setTdaxisGain(200);
}

void loop() {
    saratechUpdateSensors();

    // Rotate with manual gyro addition + servo
    // Add 5 degrees to gyro, point servo to 45 degrees
    sina.rotate(5.0, 45);
    delay(2000);

    // Rotate opposite direction
    sina.rotate(-5.0, 135);
    delay(2000);

    // Stop rotation
    sina.stop();
    delay(1000);
}
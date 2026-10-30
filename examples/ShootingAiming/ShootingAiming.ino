/**
 * @file ShootingAiming.ino
 * @brief Shooting & aiming with original Amirreza class
 * @version 2.0.0
 */

#include <saratech.h>

void setup() {
    saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
    amirreza.setTdaxisGain(200);

    // Initialize shooter if needed
    // amirreza.shoot(PC13, PA4);
}

void loop() {
    saratechUpdateSensors();

    // Face ball
    amirreza.clapm(30, 90, 120, 45, 60);

    // Face goal and shoot
    // amirreza.clapmshoot(90, 120, 90, 240);

    // Servo aim with fixed yaw
    // amirreza.shootclamp();

    // Smart shoot sequence
    // amirreza.smshoot(15, 30, 30, 45);

    // Update TDaxis gain
    amirreza.setTdaxisGain(200);
}
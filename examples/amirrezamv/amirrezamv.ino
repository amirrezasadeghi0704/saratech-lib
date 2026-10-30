#include <saratech.h>

void setup() {
    saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
    amirreza.setTdaxisGain(200);
}

void loop() {
    aghaparsa();

    // Move forward at 10 cm/s with 1 cm/s^2 shetab
    amirreza.move(10, 0, 100, 1);
    delay(2000);

    // Move right at 10 cm/s
    amirreza.move(10, 90, 0, 1);
    delay(2000);

    // Move backward at 10 cm/s
    amirreza.move(10, 0, -100, 1);
    delay(2000);

    // Move left at 10 cm/s
    amirreza.move(10, -90, 0, 1);
    delay(2000);

    // Stop
    amirreza.stop();
    delay(1000);

    amirreza.shoot(PA13,PA4);
}
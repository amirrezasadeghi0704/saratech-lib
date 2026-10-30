#include <saratech.h>

void setup() {
    saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
    amirreza.setTdaxisGain(200);
}

void loop() {
    aghaparsa();

    sina.rotate(5.0, 45);
    delay(2000);

    sina.rotate(-5.0, 135);
    delay(2000);

    sina.stop();
    delay(1000);
}
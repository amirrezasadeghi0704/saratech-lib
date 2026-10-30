#include <saratech.h>

void setup() {
    saratechBeginGTD35(&Wire, 0x10, Serial1, -1, HIGH, 115200);
    amirreza.setTdaxisGain(200);
}

void loop() {
    aghaparsa();

    // Go to position (90, 120) at 15 cm/s
    sadra.move(90, 120, 15);
    delay(5000);

    // Return to origin
    king.forati(15, 0, 0);
    delay(5000);
}
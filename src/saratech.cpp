#include "saratech.h"

// Global instances
Sina sina;
Sadra sadra;
Amirreza amirreza;
King king;
MotorDriver motorDriver;
GTD35Adapter gtd35Adapter;
TDAxis12Adapter tdaAxis12;
ServoAdapter servoAdapter;

void saratechBegin(TwoWire* i2cWire, uint8_t tdaAddress) {
    // Initialize I2C for TDAxis12
    tdaAxis12.begin(i2cWire, tdaAddress);

    // Initialize motor driver (PWM mode - default)
    motorDriver.begin();
    sina.begin(&motorDriver);
}

// Alternative initialization for GTD35 UART mode
void saratechBeginGTD35(TwoWire* i2cWire, uint8_t tdaAddress, HardwareSerial& serialPort, int8_t directionPin = -1, uint8_t txLevel = HIGH, uint32_t baud = 115200) {
    // Initialize I2C for TDAxis12
    tdaAxis12.begin(i2cWire, tdaAddress);

    // Initialize GTD35 motor driver (UART mode)
    gtd35Adapter.begin(serialPort, baud, directionPin, txLevel);
    sina.begin(&gtd35Adapter);

    // Initialize servo (PB10)
    servoAdapter.begin();
    amirreza.begin(&sina, &servoAdapter);
    king.begin(&sina);

    // Initialize Sadra
    sadra.begin(&sina);

    // Initialize shooter (PC13 command, PA4 feedback)
    amirreza.shoot(PC13, PA4);

    // Read distance sensor baselines (average 10 samples each)
    // This matches the original sketch initialization
    int kf_sum = 0, kr_sum = 0, kb_sum = 0, kl_sum = 0;
    for (int i = 0; i < 10; i++) {
        kf_sum += analogRead(PA5);
        kr_sum += analogRead(PA6);
        kb_sum += analogRead(PA7);
        kl_sum += analogRead(PB0);
        delay(5);
    }
    RobotState::instance().setBaselines(
        kf_sum / 10,
        kr_sum / 10,
        kb_sum / 10,
        kl_sum / 10
    );

    // Initialize TSOP channel selector pins
    pinMode(PA8, OUTPUT);
    pinMode(PB1, OUTPUT);
    pinMode(PC14, OUTPUT);
    pinMode(PC15, OUTPUT);
}

// Helper function to read all sensors (call in main loop)
void saratechUpdateSensors() {
    RobotState& state = RobotState::instance();

    // Read TDAxis12 yaw
    float yaw = tdaAxis12.read();

    // Read distance sensors
    if (state.baselinesSet) {
        int rawFront = analogRead(PA5);
        int rawRight = analogRead(PA6);
        int rawBack = analogRead(PA7);
        int rawLeft = analogRead(PB0);

        float distFront = state.adcToCm(rawFront, state.baselineFront);
        float distRight = state.adcToCm(rawRight, state.baselineRight);
        float distBack = state.adcToCm(rawBack, state.baselineBack);
        float distLeft = state.adcToCm(rawLeft, state.baselineLeft);

        state.updateDistances(distFront, distRight, distBack, distLeft,
                              distFront > 0 && distFront < 300,
                              distRight > 0 && distRight < 300,
                              distBack > 0 && distBack < 300,
                              distLeft > 0 && distLeft < 300);
    }

    // Read TSOP ball sensor (16 channels)
    float ballAngle, ballDistance;
    if (tdaAxis12.getBallData(ballAngle, ballDistance)) {
        bool ballValid = ballDistance > 2000.0f;
        bool inKicker = false;

        // Ball in kicker: PA4 LOW + ball distance > 7500 + angle -90 to 90
        if (digitalRead(PA4) == LOW && ballDistance > 7500.0f &&
            ballAngle > -90.0f && ballAngle < 90.0f) {
            inKicker = true;
        }

        state.updateBall(ballAngle, ballDistance, ballValid, inKicker);
    }

    // Read side sensors
    int shr = analogRead(PA2);
    int shl = analogRead(PA3);
    int shb = analogRead(PA1);
    state.updateSideSensors(shr, shl, shb);

    // Update position estimate using Sadra's mapcalc
    sadra.mapcalc(state.distBack, state.distFront, state.distLeft, state.distRight);
}
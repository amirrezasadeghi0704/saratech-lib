#include "saratech.h"

// ============================================================================
// GLOBAL INSTANCES (4 Components Only)
// ============================================================================

// Component 1: TDAxis12 Adapter
TDAxis12Adapter tdaAxis12(&Wire, 0x10);

// Component 2: GTD35 Adapter
GTD35Adapter gtd35Adapter;

// Component 3: Servo Adapter
ServoAdapter servoAdapter(PB10);

// Component 4: Spin Back Motor
SpinBackMotor spinBackMotor(PB11, PB12);

// ============================================================================
// NEW CLASS INSTANCES
// ============================================================================

amirreza_class amirreza;      // Gyro coeff + basic movement
sina_class sina;              // Rotation + servo
sadra_class sadra;            // Go to position
king_forati_class king;       // Return to start

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void saratechBeginGTD35(TwoWire* i2cWire, uint8_t tdaAddress,
                        HardwareSerial& serialPort,
                        int8_t directionPin, uint8_t txLevel,
                        uint32_t baud) {
    // Initialize TDAxis12 (I2C)
    tdaAxis12.begin(i2cWire, tdaAddress);

    // Initialize GTD35 (UART)
    gtd35Adapter.begin(serialPort, baud, directionPin, txLevel);

    // Initialize Servo
    servoAdapter.begin();

    // Initialize Spin Back Motor
    spinBackMotor.begin();

    // Initialize classes with dependencies
    amirreza.begin(&gtd35Adapter, &spinBackMotor);
    sina.begin(&gtd35Adapter, &servoAdapter);
    sadra.begin(&gtd35Adapter, &amirreza);
    king.begin(&amirreza);

    // Read distance sensor baselines (average 10 samples each)
    int kf_sum = 0, kr_sum = 0, kb_sum = 0, kl_sum = 0;
    for (int i = 0; i < 10; i++) {
        kf_sum += analogRead(PA5);
        kr_sum += analogRead(PA6);
        kb_sum += analogRead(PA7);
        kl_sum += analogRead(PB0);
        delay(5);
    }
    RobotState::instance().setBaselines(
        kf_sum / 10, kr_sum / 10, kb_sum / 10, kl_sum / 10
    );

    // Initialize TSOP channel selector pins
    pinMode(PA8, OUTPUT);
    pinMode(PB1, OUTPUT);
    pinMode(PC14, OUTPUT);
    pinMode(PC15, OUTPUT);
}

// Update all sensors (call in main loop)
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

    // Update position estimate
    sadra.mapcalc(state.distBack, state.distFront, state.distLeft, state.distRight);
}

namespace saratech {
    inline float normalizeAngle(float angle) {
        while (angle > 180.0f) angle -= 360.0f;
        while (angle < -180.0f) angle += 360.0f;
        return angle;
    }
    inline float rad(float deg) { return deg * PI / 180.0f; }
    inline float deg(float rad) { return rad * 180.0f / PI; }
    template<typename T>
    T clamp(T val, T minVal, T maxVal) {
        return val < minVal ? minVal : (val > maxVal ? maxVal : val);
    }
}
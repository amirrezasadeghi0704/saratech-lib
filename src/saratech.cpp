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
    sadra.begin(&amirreza);
    king.begin(&amirreza);

}

// Update sensors from 4 allowed hardware only: TDAxis (yaw + ball)
void aghaparsa() {
    RobotState& state = RobotState::instance();

    // Read TDAxis12/13 yaw (I2C)
    float yaw = tdaAxis12.read();

    // Read ball data from TDAxis TSOP (part of TDAxis hardware)
    float ballAngle, ballDistance;
    if (tdaAxis12.getBallData(ballAngle, ballDistance)) {
        bool ballValid = ballDistance > 2000.0f;
        state.updateBall(ballAngle, ballDistance, ballValid, false);
    }
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

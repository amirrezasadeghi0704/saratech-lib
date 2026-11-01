#ifndef SARATECH_OP_H
#define SARATECH_OP_H

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
#include <math.h>

// Internal library headers (all live in src/ subdirectories)
#include "GTD35_Custom/GTD35.h"
#include "TDAxis12/TDAxis12.h"
#include "STM32_ISR_Servo/STM32_ISR_Servo.h"
#include "Adafruit_GFX/Adafruit_GFX.h"
#include "Adafruit_SH1106_STM32/Adafruit_SH1106_STM32.h"

// ── Pin constants (from Secondary-league PCB / Pinout.txt) ──────────────────
// Motor driver (GTD35) — UART1
#define SARATECH_MOTOR_UART      Serial1
#define SARATECH_MOTOR_RX_PIN    PA10
#define SARATECH_MOTOR_TX_PIN    PA9
#define SARATECH_MOTOR_DE_PIN    -1   // -1 = no RS485 direction pin

// IMU (TDAxis12) — I2C2
#define SARATECH_IMU_I2C         Wire2
#define SARATECH_IMU_SDA         PB11
#define SARATECH_IMU_SCL         PB10
#define SARATECH_IMU_ADDR        0x10

// ── Analog proximity sensors ────────────────────────────────────────────────
#define SARATECH_SNSR_BOTTOM     PA1
#define SARATECH_SNSR_RIGHT      PA2
#define SARATECH_SNSR_LEFT       PA3

// LDRs (4 directions) — present in both Secondary and Open-Weight
#define SARATECH_LDR_FRONT       PA5
#define SARATECH_LDR_RIGHT       PA6
#define SARATECH_LDR_BACK        PA7
#define SARATECH_LDR_LEFT        PB0

// Kicker / shooter
#define SARATECH_KICKER_DETECT   PA4   // active-LOW when ball present
#define SARATECH_KICKER_TRIGGER  PC13  // active-HIGH pulse to fire

// Servo pins (ISR-based via STM32_ISR_Servo)
#define SARATECH_SERVO_PIN       PA15
#define SARATECH_SPINBACK_PIN    PB5

// OLED SH1106 (I2C2 on PB10/PB11, address 0x3C)
#define SARATECH_OLED_I2C        Wire2
#define SARATECH_OLED_ADDR       0x3C
#define SARATECH_OLED_RST        -1

// ── Default wheel geometry ───────────────────────────────────────────────────
// Wheel diameter confirmed: 5 cm
#define SARATECH_DEFAULT_WHEEL_DIAMETER_CM  5.0f
#define SARATECH_DEFAULT_WHEEL_RADIUS_CM    2.5f
// Wheelbase (front-rear centre distance) — UNVERIFIED, needs mechanical measurement
#define SARATECH_DEFAULT_WHEELBASE_CM       16.0f
// Track width (left-right centre distance) — UNVERIFIED, needs mechanical measurement
#define SARATECH_DEFAULT_TRACK_CM           16.0f
// Max motor speed in RPM (GTD35 default)
#define SARATECH_DEFAULT_MAX_MOTOR_RPM      1000.0f

class SaratechOP {
public:
    // ── Configuration structs ─────────────────────────────────────────────
    struct MotorConfig {
        HardwareSerial *serial   = &Serial1;
        int8_t            dirPin = SARATECH_MOTOR_DE_PIN;
        uint32_t          baud   = 115200;
        // GTD35 motor IDs (FL=3, FR=0, RR=1, RL=2)
        uint8_t           idFL   = 3;
        uint8_t           idFR   = 0;
        uint8_t           idRR   = 1;
        uint8_t           idRL   = 2;
        float             wheelRadius_cm = SARATECH_DEFAULT_WHEEL_RADIUS_CM;
        float             wheelBase_cm   = SARATECH_DEFAULT_WHEELBASE_CM;
        float             track_cm       = SARATECH_DEFAULT_TRACK_CM;
        float             maxSpeed_cms   = 200.0f;
        float             maxAccel_cms2  = 100.0f;
        float             maxMotorRPM    = SARATECH_DEFAULT_MAX_MOTOR_RPM;
        uint16_t          txAxisDefault  = 4000;
        float             txAxisGain     = 1.0f;
    };

    struct SensorConfig {
        TwoWire          *i2c      = &Wire2;
        uint8_t           imuAddr  = SARATECH_IMU_ADDR;
        int               ldrThresh = 300;
        int               ldrBasis[4] = {0, 0, 0, 0}; // [front, right, back, left]
        float             kickerThresh  = 7500.0f;
        float             kickerAngleLim = 90.0f;
    };

    struct ShooterConfig {
        uint8_t  triggerPin  = SARATECH_KICKER_TRIGGER;
        uint8_t  servoPin    = SARATECH_SERVO_PIN;
        uint8_t  spinbackPin = SARATECH_SPINBACK_PIN;
        bool     spinbackIsPwm = true;
        float    servoMinDeg  = 0.0f;
        float    servoMaxDeg  = 180.0f;
        uint16_t servoMinUs   = 500;
        uint16_t servoMaxUs   = 2400;
        uint8_t  spinbackMax  = 255;
    };

    struct MotionConfig {
        uint16_t tdAxisCoeff = 200;     // 0-512, scales gyro compensation
        float    yawOffsetDeg = 0.0f;
        float    posTol_cm    = 5.0f;
        float    angTol_deg   = 5.0f;
        float    shiftCoeff   = 1.0f;   // curved-approach coefficient
    };

    // ── Constructors / begin ──────────────────────────────────────────────
    SaratechOP();
    ~SaratechOP();

    void begin();
    void begin(const MotorConfig &mc, const SensorConfig &sc,
               const ShooterConfig &shc, const MotionConfig &moc);

    // ── Wheel geometry setters ─────────────────────────────────────────────
    void setWheelDiameter(float cm);   // diameter in cm (default 5.0)
    void setWheelRadius(float cm);     // radius in cm (default 2.5)
    void setWheelBase(float cm);       // front-rear distance
    void setTrackWidth(float cm);      // left-right distance

    // ── Speed / accel limits ───────────────────────────────────────────────
    void setMaxSpeed(float cms);
    void setMaxAccel(float cms2);
    void setMaxMotorRPM(float rpm);

    // ── TDAXIS / TXAXIS ───────────────────────────────────────────────────
    void setTDAxisCoefficient(uint16_t coeff);  // 0-512
    void setTDAxis(uint16_t coeff);             // alias
    void setTXAxisCoefficient(uint16_t coeff);  // alias
    void setTXAxisGain(float gain);
    void setTXAxisAngle(uint16_t angle);        // raw angle sent to GTD35

    // ── Sensor config setters ──────────────────────────────────────────────
    void setLDRThreshold(int threshold);
    void setLDRBaseline(int idx, int value);   // idx: 0=front,1=right,2=back,3=left
    void setKickerThreshold(float dist, float angleDeg);
    void setServoLimits(uint16_t minUs, uint16_t maxUs, float minDeg, float maxDeg);
    void setSpinbackMaxSpeed(uint8_t speed);
    void setPositionTolerance(float cm);
    void setAngleTolerance(float deg);
    void setShiftCoefficient(float c);

    // ── Five main API functions ───────────────────────────────────────────

    // 1. Move with physical units
    //    speed_cm_s: linear speed (positive = forward, negative = reverse)
    //    accel_cm_s2: linear acceleration clamp
    //    direction_deg: heading in robot frame (0=forward, 90=right, -90=left)
    bool move(float speed_cm_s, float accel_cm_s2, float direction_deg);

    // 2. Coordinate-based movement
    //    targetX_cm, targetY_cm: target in robot-local cm (X=right, Y=forward)
    //    speed_cm_s: movement speed
    //    Returns true when target reached within tolerance
    bool goTo(float targetX_cm, float targetY_cm, float speed_cm_s);

    // 3. Update TXAXIS compensation (read IMU, apply correction)
    void updateTXAXISCompensation();

    // 4A. Set TDAXIS coefficient
    // 4B. Intentional yaw rotation / reset
    void rotateYaw(float angleDeg);
    void resetYaw();

    // 4C. Shoot
    //    servoAngle_deg: shooter arm angle
    //    spinBackSpeed: 0-255
    void shoot(float servoAngle_deg, uint8_t spinBackSpeed);

    // 5. Curved approach (shift)
    //    Returns the adjusted heading angle in degrees
    float shift(float robotX_cm, float robotY_cm,
                float ballX_cm, float ballY_cm,
                float shiftCoeff);

    // ── Sensor polling ─────────────────────────────────────────────────────
    /** Update all sensors: IMU + LDRs + analog proximity + kicker. */
    void updateSensors();
    /** Read TDAxis12 IMU (yaw). */
    void updateIMU();
    /** Read 4-direction LDR sensors. */
    void updateLDR();
    /** Read analog proximity sensors (bottom/right/left). */
    void updateAnalogSensors();
    /** Check kicker mechanism state. */
    void updateKicker();

    // ── Low-level motor control ───────────────────────────────────────────
    bool setMotorSpeeds(int16_t fl, int16_t fr, int16_t rr, int16_t rl);
    bool setMotorSpeedsRPM(float fl_rpm, float fr_rpm, float rr_rpm, float rl_rpm);
    void stopMotors();

    // ── Servo helpers ─────────────────────────────────────────────────────
    void setShooterServoAngle(float deg);
    void setSpinbackSpeed(uint8_t pwm);

    // ── Public state getters ──────────────────────────────────────────────
    float    getRobotYaw()        const { return _yaw; }
    float    getRobotYawRaw()     const { return _yawRaw; }
    float    getRobotX()          const { return _posX; }
    float    getRobotY()          const { return _posY; }
    bool     isBallInKicker()     const { return _ballInKicker; }
    uint16_t getTDAxisCoeff()     const { return _motion.tdAxisCoeff; }
    float    getYawOffset()       const { return _motion.yawOffsetDeg; }
    float    getMaxSpeed()        const { return _motor.maxSpeed_cms; }
    float    getMaxAccel()        const { return _motor.maxAccel_cms2; }
    float    getWheelRadius()     const { return _motor.wheelRadius_cm; }
    float    getWheelBase()       const { return _motor.wheelBase_cm; }
    float    getTrackWidth()      const { return _motor.track_cm; }

    // LDR state getters
    bool getLDRFront()  const { return _ldrFront; }
    bool getLDRRight()  const { return _ldrRight; }
    bool getLDRBack()   const { return _ldrBack; }
    bool getLDRLeft()   const { return _ldrLeft; }

    // Analog sensor getters
    int  getBottomSensor() const { return _snsrBottom; }
    int  getRightSensor()  const { return _snsrRight; }
    int  getLeftSensor()   const { return _snsrLeft; }

    // Direct hardware access
    GTD35&          gtd35()       { return _gtd35; }
    TDAxis12&       imu()         { return _imu; }
    STM32_ISR_Servo& servoCtrl()   { return _servo; }
    Adafruit_SH1106& display()     { return _display; }

private:
    // ── Hardware instances ─────────────────────────────────────────────────
    GTD35            _gtd35;
    TDAxis12         _imu;
    STM32_ISR_Servo  _servo;
    Adafruit_SH1106  _display;

    // ── Configuration ─────────────────────────────────────────────────────
    MotorConfig  _motor;
    SensorConfig _sensor;
    ShooterConfig _shooter;
    MotionConfig _motion;

    // ── Runtime state ─────────────────────────────────────────────────────
    bool  _ballInKicker  = false;

    float _yaw           = 0.0f;   // compensated yaw
    float _yawRaw        = 0.0f;   // raw from IMU
    float _posX          = 0.0f;   // odometry X (cm, right-positive)
    float _posY          = 0.0f;   // odometry Y (cm, forward-positive)

    // Sensor readings
    int   _snsrBottom    = 0;
    int   _snsrRight     = 0;
    int   _snsrLeft      = 0;
    bool  _ldrFront      = false;
    bool  _ldrRight      = false;
    bool  _ldrBack       = false;
    bool  _ldrLeft       = false;

    // Servo index
    int8_t _servoIdx     = -1;

    // Speed tracking (for acceleration ramping)
    float _lastSpeed_cms = 0.0f;
    uint32_t _lastMoveMs = 0;
    bool     _initialized = false;

    // ── Private helpers ───────────────────────────────────────────────────
    void computeWheelSpeeds(float vx, float vy, float omega,
                            int16_t &fl, int16_t &fr,
                            int16_t &rr, int16_t &rl);
    void applyTXAXISComp(int16_t &fl, int16_t &fr,
                         int16_t &rr, int16_t &rl);
    void applyTDAXISInfluence(int16_t &fl, int16_t &fr,
                              int16_t &rr, int16_t &rl);

    float normAngle(float a);
    float cmsToRPM(float cms) const;
    float rpmToCMS(float rpm) const;

    void updateOdometry(float vx, float vy, float dt_s);
};

// Global singleton
extern SaratechOP saratech;

#endif // SARATECH_OP_H

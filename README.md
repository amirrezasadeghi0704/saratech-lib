# SARATECH Arduino Robotics Library

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/en/Reference/Libraries)
[![Platform](https://img.shields.io/badge/Platform-STM32-red.svg)](https://github.com/stm32duino/Arduino_Core_STM32)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/Version-1.0.0-orange.svg)]()

Complete Arduino library for **four-wheel omnidirectional football robots** used in IranOpen Secondary competitions. Provides high-level robot control interfaces for movement, field mapping, ball detection, goal alignment, shooting, and navigation.

---

## 🎯 Features

| Module | Class | Description |
|--------|-------|-------------|
| **Motion** | `Sina` | Omnidirectional movement with vector control & yaw correction |
| **Field Mapping** | `Sadra` | Position estimation from 4 distance sensors + coordinate navigation |
| **Shooting & Aiming** | `Amirreza` | Ball tracking, goal alignment, servo turret, smart shooting state machine |
| **Return** | `parsa` | Navigate back to start position with distance-proportional speed |

### 9 Public APIs
```cpp
// Motion
sina.motor(speed, x, y, tdaxisGain, acceleration);

// Field mapping & navigation
sadra.mapcalc(rear, front, left, right);
sadra.move(targetX, targetY, maxSpeed);

// Ball & goal alignment
amirreza.clapm(maxRotation, robotX, robotY, ballX, ballY);
amirreza.clapmshoot(robotX, robotY, goalX, goalY);

// Servo turret aiming
amirreza.shootclamp();

// Smart combined shooting
amirreza.smshoot(approachSpeed, minGoalDist, maxRobotYaw, maxServoYaw);

// Return to start
king.forati(maxSpeed, targetX, targetY);
```

---

## 🔧 Hardware Requirements

| Component | Specification | Pins |
|-----------|--------------|------|
| **MCU** | STM32F103C8T6 (Blue Pill) | — |
| **Motors** | 4× GTD35 (UART/RS485 only) | Serial1/2/3 (TX/RX + optional DE pin) |
| **IMU** | TDAxis12 (I²C @ 0x10) | I2C2: PB10(SCL), PB11(SDA) |
| **Distance Sensors** | 4× Analog (field lines) | Front: PA5, Right: PA6, Back: PA7, Left: PB0 |
| **Ball Sensor** | 16-ch TSOP array | PA0 + PA8, PB1, PC14, PC15 (selectors) |
| **Shooter** | Solenoid/Trigger | Cmd: PC13 (HIGH), Feedback: PA4 (LOW=ready) |
| **Servo** | Turret (45°–135°) | PB10 |

> **⚠️ The GTD35 motors operate ONLY via UART/RS485 protocol (115200 baud). There is no PWM mode. The library uses the native GTD35 UART implementation.**

---

## 📦 Installation

### Arduino IDE
1. Download this repository as ZIP
2. **Sketch → Include Library → Add .ZIP Library**
3. Select `saratech-main.zip`
4. Restart Arduino IDE

### Manual
```bash
# Copy to Arduino libraries folder
cp -r saratech ~/Documents/Arduino/libraries/
```

### PlatformIO
```ini
lib_deps = 
    https://github.com/amirrezasadeghi0704/saratech-lib.git
```

---

## 🚀 Quick Start

```cpp
#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);
    // Initialize with GTD35 UART on Serial1, no direction pin, 115200 baud
    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);
}

void loop() {
    saratechUpdateSensors();     // MUST call every loop iteration
    
    // Move right at 10 cm/s
    sina.motor(10, 90, 0, 200, 0);
    
    // Move to field coordinates (90, 120) at 15 cm/s
    sadra.move(90, 120, 15);
    
    // Align to opponent goal and shoot
    amirreza.clapmshoot(robotX, robotY, 90, 240);
}
```

---

## 📚 API Reference

### `Sina` — Omnidirectional Motion
```cpp
void motor(float speed, float x, float y, float tdaxisGain=200, float acceleration=0);
void stop();
void getVelocity(float& vx, float& vy);
bool isMoving();
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `speed` | cm/s | Linear speed (max 30) |
| `x, y` | — | Direction vector components |
| `tdaxisGain` | — | Yaw gain multiplier (200=default, 0=disabled) |
| `acceleration` | cm/s² | Accel limit (0=unlimited) |

**Example:** `sina.motor(10, 90, 0, 200, 0)` → move right at 10 cm/s

---

### `Sadra` — Field Mapping & Navigation
```cpp
void mapcalc(float rear, float front, float left, float right);
bool move(int targetX, int targetY, int maxSpeed);
bool isMoving();
float distanceToTarget();
void stop();
```
- **Field**: 180×240 cm, origin at our goal bottom-left (0,0)
- **Arrival tolerance**: 5 cm

---

### `Amirreza` — Shooting & Aiming
```cpp
void shoot(uint8_t cmdPin, uint8_t fbPin);           // Initialize shooter
void clapm(float maxRot, float rx, float ry, float bx, float by);  // Face ball
void clapmshoot(float rx, float ry, float gx, float gy);           // Face goal & shoot
void shootclamp();                                    // Servo aim + fixed yaw
void smshoot(int speed, int n, int i, int z);        // Smart shoot sequence
bool checkBallReady();
const char* getShooterStateStr();
void resetSmshoot();
```
| `smshoot` Param | Meaning |
|----------------|---------|
| `speed` | Approach speed (cm/s) |
| `n` | Min distance from goal (cm) |
| `i` | Max robot yaw rotation (°) |
| `z` | Max servo rotation from center (°) |

---

### `King` — Return to Start
```cpp
void forati(int maxSpeed, int targetX, int targetY);
bool isReturning();
bool atTarget();
float distanceToTarget(int x, int y);
void stop();
```
- Uses distance-proportional speed: `v = min(v_max, k*d)`

---

### Library Initialization
```cpp
// GTD35 UART mode (native - only supported mode)
void saratechBeginGTD35(TwoWire* i2cWire, uint8_t tdaAddress,
                        HardwareSerial& serialPort,
                        int8_t directionPin = -1,
                        uint8_t txLevel = HIGH,
                        uint32_t baud = 115200);

// Sensor update (call every loop)
void saratechUpdateSensors();
```

---

## 🤖 Sensor Update Loop

**Critical:** Call `saratechUpdateSensors()` once per `loop()` iteration:
```cpp
void loop() {
    saratechUpdateSensors();  // Updates yaw, distances, ball, position
    
    // Your robot logic here
}
```

This reads:
- TDAxis12 yaw (I²C)
- 4 distance sensors (analog, calibrated at startup)
- 16-channel TSOP ball sensor
- Side sensors for yaw offset (`dgy`)
- Updates position estimate via `Sadra`

---

## 📂 Examples

| Example | Description |
|---------|-------------|
| `BasicMovement` | Vector-based motion in all directions |
| `FieldMapping` | Position estimation & coordinate movement |
| `GoalAlignment` | Ball tracking & goal-facing rotation |
| `Shooting` | All shooting modes (direct, servo, smart) |
| `FullMatch` | Complete match state machine |

Open via **File → Examples → SARATECH**

---

## ⚙️ Configuration

Adjust in `RobotState::Config` (via `RobotState::instance().config`):

```cpp
config.maxSpeedCmS = 30.0f;        // Max linear speed
config.maxMotorCmd = 32767;        // Max command (GTD35 UART mode, int16_t)
config.yawGainBase = 300.0f;       // Yaw correction gain (motor units/°)
config.fieldWidth = 180.0f;        // Field X (cm)
config.fieldLength = 240.0f;       // Field Y (cm)
config.arrivalTolerance = 5.0f;    // Target arrival (cm)
config.shootAlignmentTol = 3.0f;   // Shooting yaw tolerance (°)
config.servoMin = 45.0f;           // Servo mechanical limits
config.servoMax = 135.0f;
config.servoCenter = 90.0f;
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────┐
│           RobotState (Singleton)    │
│  yaw, position, distances, ball,    │
│  servo, motor cmds, errors, config  │
└──────────────┬──────────────────────┘
               │
    ┌──────────┼──────────┬───────────┐
    ▼          ▼          ▼           ▼
┌───────┐ ┌────────┐ ┌──────────┐ ┌────────┐
│ Sina  │ │ Sadra  │ │ Amirreza │ │ King   │
│motion │ │map/nav │ │ shoot/aim│ │ return │
└───┬───┘ └────┬───┘ └────┬─────┘ └────┬───┘
    │          │          │           │
    ▼          ▼          ▼           ▼
┌─────────────────────────────────────┐
│        Hardware Adapters            │
│ GTD35Adapter │ TDAxis12Adapter │ ServoAdapter │
└─────────────────────────────────────┘
```

### Motor Control (GTD35 UART Only)
| Mode | Class | Transport | Max Command | Protocol |
|------|-------|-----------|-------------|----------|
| **UART** | `GTD35Adapter` | Serial/RS485 | 32767 | Native GTD35 |

**Protocol Details:**
- Frame: `0xAA 0xAA | len=4 | 4 motor IDs | per-motor: control(1B) + speed(2B) + angle(2B) | XOR checksum`
- Motor order: L1, L2, R2, R1 (ML1, ML2, MR2, MR1)
- Speed: int16_t (-32768 to 32767) representing RPM
- Angle: uint16_t (default 4000)
- Control byte: enable(0x0F) | resetEncoder(0xF0)
- Baud: 115200 default

---

## 📄 License

MIT License - see [LICENSE](LICENSE) for details.

Based on:
- [TDAxis12](https://github.com/fdmxfarhan/TDAxis12) by fdmxfarhan
- [GTD35](https://github.com/fdmxfarhan/GTD35) by fdmxfarhan

---

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/amirrezasadeghi0704/saratech-lib/issues)
- **IranOpen Secondary**: Student football robotics competition

---

**Made for IranOpen 1405 Secondary Football Robotics** ⚽🤖

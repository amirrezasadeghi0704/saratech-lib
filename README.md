# SARATECH Arduino Robotics Library v2.0.0

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/en/Reference/Libraries)
[![Version](https://img.shields.io/badge/Version-2.0.0-orange.svg)]()

Complete Arduino library for **Open Weight league soccer robots**. Preserves all TDaxis12 or 13 and GTD35 commands while adding new consolidated classes with simplified naming.

---

## 🎯 Version 2.0.0 - What's New

| Feature | Description |
|---------|-------------|
| **Version** | 2.0.0 (Open Weight League) |
| **Wheel Radius** | Fixed at 5 cm for all robots |
| **Components** | Spin Back Motor PWM, GTD35, TDAxis, Servo |
| **All TDaxis12 commands** | ✅ Preserved |
| **All GTD35 commands** | ✅ Preserved |
| **New Classes** | `amirreza_class`, `sina_class`, `sadra_class`, `king.forati_class`, `sina_rotation_class` |

---

## 🎯 Features

| Module | Class | Description |
|--------|-------|-------------|
| **TDAxis Coefficient** | `amirreza_class` | Receive TDaxis coefficient for all movement functions |
| **Vector Control** | `sina_class` | cm/s vector control with acceleration (3 inputs: speed, x, y) + acceleration (1 input) |
| **Go-to-Point** | `sadra_class` | Move to field coordinates in cm/s (3 inputs: targetX, targetY, speed) |
| **Return to Start** | `forati_class` | Return to start position (3 inputs: speed, xtarg, ytarg) |
| **Rotation + Servo** | `sina_rotation_class` | Manual gyro addition + servo control (2 inputs) |

---

## 🔗 Original Libraries (Credits)

All TDaxis12 and GTD35 commands are preserved from the original libraries:

| Original Library | Author | GitHub |
|------------------|--------|--------|
| **TDAxis12** | fdmxfarhan | https://github.com/fdmxfarhan/TDAxis12 |
| **GTD35** | fdmxfarhan | https://github.com/fdmxfarhan/GTD35 |

---

## 🎯 Features

| Module | Class | Description |
|--------|-------|-------------|
| **TDAxis Coefficient** | `amirreza_class` | Receive TDaxis coefficient for all movement functions |
| **Vector Control** | `sina_class` | cm/s vector control with acceleration (3 inputs: speed, x, y) + acceleration (1 input) |
| **Go-to-Point** | `sadra_class` | Move to field coordinates in cm/s (3 inputs: targetX, targetY, speed) |
| **Return to Start** | `forati_class` | Return to start position (3 inputs: speed, xtarg, ytarg) |
| **Rotation + Servo** | `sina_rotation_class` | Manual gyro addition + servo control (2 inputs) |

---

## 📦 Installation

### Arduino IDE
1. Download as ZIP
2. **Sketch → Include Library → Add .ZIP Library**
3. Select `saratech.zip`

### Manual
```bash
cp -r saratech ~/Documents/Arduino/libraries/
```

---

## 🚀 Quick Start

```cpp
#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);
    
    // Initialize with GTD35 UART on Serial1
    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);
}

void loop() {
    saratechUpdateSensors();  // MUST call every loop
    
    // 1. sina_class: Vector control with cm/s + acceleration
    sina.motor(10, 90, 0, 200, 5);  // 10 cm/s right, accel 5 cm/s²
    
    // 2. sadra_class: Go to point (3 inputs)
    sadra.move(90, 120, 15);  // Go to (90,120) at 15 cm/s
    
    // 3. king.forati_class: Return to start (3 inputs)
    king.forati(15, 0, 0);  // Return to origin at 15 cm/s
    
    // 4. amirreza_class: Set TDaxis gain for all movement
    amirreza.setTdaxisGain(250);  // Custom gain
    
    // 5. sina_rotation_class: Manual gyro + servo
    sina_rotation.rotate(5.0, 45);  // Add 5° to gyro, servo to 45°
    
    // 0. amirreza_class: Set TDaxis gain (applies to ALL movement)
    amirreza.setTdaxisGain(200);  // Default 200
}
```

---

## 📚 API Reference

### 0. `amirreza_class` — TDAxis Coefficient Receiver
```cpp
void setTdaxisGain(float gain);      // Set gain for ALL movement functions
float getTdaxisGain() const;
void enableYawCorrection(bool enable);
bool isYawCorrectionEnabled() const;
```
**Purpose:** Receives TDaxis coefficient applied to ALL movement functions to prevent unwanted yaw rotation during movement.

---

### 1. `sina_class` — Vector Control (3+1 inputs)
```cpp
void motor(float speed_cm_s, float x, float y,
           float tdaxisGain = 200.0f, float acceleration_cm_s2 = 0.0f);
void stop();
void getVelocity(float& vx, float& vy);
bool isMoving();
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `speed_cm_s` | cm/s | Linear speed (max 30) |
| `x, y` | — | Direction vector (normalized internally) |
| `tdaxisGain` | — | Yaw gain multiplier (200=default, 0=disabled) |
| `acceleration_cm_s2` | cm/s² | Acceleration limit (0=unlimited) |

**Example:**
```cpp
sina.motor(10, 90, 0, 200, 5);   // Move right at 10 cm/s, 5 cm/s² accel
sina.motor(20, 0, 100, 0, 0);    // Move forward at 20 cm/s, no yaw correction
```

---

### 2. `sadra_class` — Go to Point (3 inputs)
```cpp
bool move(int targetX_cm, int targetY_cm, int speed_cm_s);
bool isMoving();
float distanceToTarget();
void stop();
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `targetX_cm` | cm | Target X position (field coords) |
| `targetY_cm` | cm | Target Y position (field coords) |
| `speed_cm_s` | cm/s | Max approach speed |

**Returns:** `true` when target reached (within 5cm tolerance)

**Example:**
```cpp
sadra.move(90, 120, 15);  // Go to field center at 15 cm/s
if (sadra.move(0, 0, 10)) { /* arrived at origin */ }
```

---

### 3. `forati_class` — Return to Start (3 inputs)
```cpp
void forati(int speed_cm_s, int xtarg_cm, int ytarg_cm);
bool isReturning();
bool atTarget();
float distanceToTarget(int x, int y);
void stop();
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `speed_cm_s` | cm/s | Max return speed |
| `xtarg_cm` | cm | Target X (field coords) |
| `ytarg_cm` | cm | Target Y (field coords) |

**Uses distance-proportional speed:** `v = min(v_max, k*d)` for smooth deceleration.

---

### 4. `sina_rotation_class` — Rotation + Servo (2 inputs)
```cpp
void rotate(float manual_gyro_addition, float servo_angle);
void stop();
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `manual_gyro_addition` | degrees | Added to current gyro before coefficient multiplication |
| `servo_angle` | degrees | Servo turret angle (relative to center 90°) |

**Example:**
```cpp
sina_rotation.rotate(5.0, 45);   // Add 5° to gyro reading, point servo to 45°
sina_rotation.rotate(-10, 135);  // Subtract 10°, point servo to max right
```

---

### Preserved Original Classes (All TDaxis/GTD35 Commands)

All original TDaxis12 and GTD35 commands remain available:

| Original Class | Key Methods |
|----------------|-------------|
| `Sina` | `motor()`, `stop()`, `getVelocity()` |
| `Sadra` | `mapcalc()`, `move()`, `isMoving()` |
| `Amirreza` | `clapm()`, `clapmshoot()`, `smshoot()`, `shootclamp()` |
| `King` | `forati()`, `isReturning()`, `atTarget()` |
| `TDAxis12Adapter` | `read()`, `getBallData()`, `isBallDetected()` |
| `GTD35Adapter` | `motor()`, `setMotorRaw()`, `stop()` |
| `ServoAdapter` | `setAngle()`, `setOffset()`, `getAngle()` |

---

### Sensor Update (Required Every Loop)

```cpp
void loop() {
    saratechUpdateSensors();  // Updates: yaw, distances, ball, position
    
    // Your logic here
}
```

**Reads:** TDAxis12 yaw (I²C), 4 distance sensors, 16-channel TSOP ball, side sensors, updates position via Sadra.

---

## 🔧 Configuration

```cpp
RobotState& state = RobotState::instance();

// Motor
state.config.maxSpeedCmS = 30.0f;
state.config.maxMotorCmd = 32767;  // GTD35 UART mode
state.config.yawGainBase = 300.0f;

// Field (cm)
state.config.fieldWidth = 180.0f;
state.config.fieldLength = 240.0f;
state.config.arrivalTolerance = 5.0f;

// Servo
state.config.servoMin = 45.0f;
state.config.servoMax = 135.0f;
state.config.servoCenter = 90.0f;

// Wheel radius: FIXED at 5 cm
#define WHEEL_RADIUS_CM 5.0f
```

---

## 🏗️ Architecture (4 Components Only)

```
┌─────────────────────────────────────┐
│           RobotState (Singleton)    │
│  yaw, position, distances, ball,    │
│  servo, motor cmds, errors, config  │
└──────────────┬──────────────────────┘
               │
    ┌──────────┼──────────┬───────────┐
    ▼          ▼          ▼           ▼
┌───────────┐ ┌─────────┐ ┌──────────┐ ┌─────────────┐
│ amirreza_ │ │ sina_   │ │ sadra_   │ │ forati │
│ class     │ │ class   │ │ class    │ │ _class      │
└─────┬─────┘ └────┬────┘ └────┬─────┘ └─────┬──────┘
      │            │           │            │
      └────────────┼───────────┼───────────┘
                   ▼
        ┌───────────────────┐
        │   sina_class      │
        │  (core motion)    │
        └────────┬──────────┘
                 ▼
    ┌─────────────────────────────┐
    │      Hardware (4 only)      │
    │ SpinBack | GTD35 | TDAxis   │
    │              Servo          │
    └─────────────────────────────┘
```

---

## 📦 Installation

1. Download as ZIP
2. Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
3. Select `saratech.zip`

---

**Made for IranOpen 1405 Open Weight Football Robotics** ⚽🤖

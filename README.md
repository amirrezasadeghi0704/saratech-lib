# SARATECH Arduino Robotics Library v1.4.7

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://www.arduino.cc/en/Reference/Libraries)
[![Version](https://img.shields.io/badge/Version-2.0.0-orange.svg)]()

Complete Arduino library for **Open Weight league soccer robots**. Preserves all TDaxis12 and GTD35 commands while adding new consolidated classes with clear role separation.

---

| Feature | Description |
|---------|-------------|
| **Version** | 2.0.0 (Open Weight League) |
| **Wheel Radius** | Fixed at 5 cm for all robots |
| **Components** | Spin Back Motor PWM, GTD35, TDAxis, Servo |
| **All TDaxis12/13 commands** | ✅ Preserved |
| **All GTD35 commands** | ✅ Preserved |
| **New Classes** | `amirreza_class`, `sina_class`, `sadra_class`, `king.forati_class` |

---

## 🔗 Original Libraries (Credits)

All TDaxis12/13 and GTD35 commands are preserved from the original libraries:

| Original Library | Author | GitHub |
|------------------|--------|--------|
| **TDAxis12/13** | fdmxfarhan | https://github.com/fdmxfarhan/TDAxis12 |
| **GTD35** | fdmxfarhan | https://github.com/fdmxfarhan/GTD35 |

---

## 🎯 Class Roles & Responsibilities

| Class | Role | Key Methods |
|-------|------|-------------|
| **amirreza_class** | **Gyro coefficient + Basic movement** (accel, speed, velocity vector) | `move(speed, x, y, accel)`, `setTdaxisGain()`, `stop()` |
| **sina_class** | **Rotation** with manual gyro addition + servo | `rotate(manual_gyro_add, servo_angle)`, `stop()` |
| **sadra_class** | **Go to position** (targetX, targetY, speed) | `move(targetX, targetY, speed)`, `stop()` |
| **king.forati_class** | **Return to start position** (speed, xtarg, ytarg) | `forati(speed, xtarg, ytarg)`, `stop()` |

---

## 🔗 Original Libraries (Credits)

All TDaxis12/13 and GTD35 commands are preserved from the original libraries:

| Original Library | Author | GitHub |
|------------------|--------|--------|
| **TDAxis12/13** | fdmxfarhan | https://github.com/fdmxfarhan/TDAxis12 |
| **GTD35** | fdmxfarhan | https://github.com/fdmxfarhan/GTD35 |

---

## 🎯 Class Details

### 1. `amirreza_class` — Gyro Coefficient + Basic Movement
```cpp
// Setup
amirreza.begin(&gtd35Adapter, &spinBackMotor);

// Basic movement with acceleration
amirreza.move(speed_cm_s, x, y, acceleration_cm_s2);

// Gyro coefficient for ALL movement functions
amirreza.setTdaxisGain(200);  // Default 200

// Stop
amirreza.stop();
```
| Method | Parameters | Description |
|--------|------------|-------------|
| `move()` | `speed_cm_s`, `x`, `y`, `accel` | Move with acceleration control |
| `setTdaxisGain()` | `gain` | Gyro coefficient for ALL movement |
| `getTdaxisGain()` | — | Get current gain |
| `stop()` | — | Stop all movement |

---

### 2. `sina_class` — Rotation + Servo
```cpp
sina.begin(&gtd35Adapter, &servoAdapter);

// Rotation with manual gyro addition + servo
sina.rotate(manual_gyro_addition, servo_angle);

// Stop rotation
sina.stop();
```
| Method | Parameters | Description |
|--------|------------|-------------|
| `rotate()` | `manual_gyro_add` (deg), `servo_angle` (45-135°) | Rotate + servo |
| `stop()` | — | Stop rotation |

---

### 3. `sadra_class` — Go to Position
```cpp
sadra.begin(&gtd35Adapter, &amirreza);

// Go to position (targetX, targetY in cm, speed in cm/s)
sadra.move(targetX_cm, targetY_cm, speed_cm_s);

// Check if moving
sadra.isMoving();

// Stop
sadra.stop();
```

---

### 4. `king.forati_class` — Return to Start
```cpp
king.begin(&amirreza);

// Return to position
king.forati(speed_cm_s, xtarg_cm, ytarg_cm);

// Stop
king.stop();
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
    aghaparsa();
    
    // 1. amirreza: Basic movement with acceleration
    amirreza.move(10, 90, 0, 5);  // 10 cm/s right, 5 cm/s² accel
    
    // 2. sadra: Go to position
    sadra.move(90, 120, 15);  // Go to (90,120) at 15 cm/s
    
    // 3. king: Return to start
    king.forati(15, 0, 0);  // Return to origin at 15 cm/s
    
    // 4. sina: Rotation + servo
    sina.rotate(5.0, 45);  // Add 5° to gyro, servo to 45°
    
    // Gyro coefficient for ALL movement
    amirreza.setTdaxisGain(200);
}
```

---

## 📚 API Reference

### `amirreza_class` — Gyro Coefficient + Basic Movement
```cpp
void move(float speed_cm_s, float x, float y, float acceleration_cm_s2 = 0);
void stop();
void setTdaxisGain(float gain);      // Gyro coefficient for ALL movement
float getTdaxisGain() const;
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `speed_cm_s` | cm/s | Linear speed (max 30) |
| `x, y` | — | Direction vector (normalized) |
| `acceleration` | cm/s² | Acceleration limit (0=unlimited) |

---

### `sina_class` — Rotation + Servo
```cpp
void rotate(float manual_gyro_addition, float servo_angle);
void stop();
```
| Parameter | Unit | Description |
|-----------|------|-------------|
| `manual_gyro_addition` | deg | Added to gyro BEFORE coefficient |
| `servo_angle` | deg | Servo angle (45=left, 90=center, 135=right) |

---

### `sadra_class` — Go to Position
```cpp
bool move(int targetX_cm, int targetY_cm, int speed_cm_s);
void stop();
```

---

### `king.forati_class` — Return to Start
```cpp
void forati(int speed_cm_s, int xtarg_cm, int ytarg_cm);
void stop();
```

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

// Wheel radius: FIXED at 5 cm
#define WHEEL_RADIUS_CM 5.0f
```

---

## 🚀 Quick Start

```cpp
#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);
    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);
}

void loop() {
    aghaparsa();  // Updates TDaxis, distances, ball, position
    
    // Basic movement with acceleration
    amirreza.move(10, 90, 0, 5);  // 10 cm/s right, 5 cm/s² accel
    
    // Go to position
    sadra.move(90, 120, 15);  // Go to (90,120) at 15 cm/s
    
    // Return to start
    king.forati(15, 0, 0);  // Return to origin
    
    // Rotation + servo
    sina.rotate(5.0, 45);  // Add 5° to gyro, servo 45°
    
    // Gyro coefficient for ALL movement
    amirreza.setTdaxisGain(200);
}
```

---

## 🔧 Hardware & Sensor Update

```cpp
void loop() {
    aghaparsa();  // Updates TDaxis (yaw + ball)
    
    // Your logic here
}
```

**Updates via `aghaparsa()`:** TDaxis12/13 yaw (I²C).

---

## 🔗 Original Libraries

| Library | Author | GitHub |
|---------|--------|--------|
| **TDAxis12/13** | fdmxfarhan | https://github.com/fdmxfarhan/TDAxis12 |
| **GTD35** | fdmxfarhan | https://github.com/fdmxfarhan/GTD35 |

---

## 📦 Installation

1. Download as ZIP
2. Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
3. Select `saratech.zip`

---

**Made for IranOpen and RoboCup Open Weight Football Robotics** ⚽🤖

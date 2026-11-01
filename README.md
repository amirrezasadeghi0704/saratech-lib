# Saratech OP

### STM32 Open-Weight Soccer Robot Control Library

[![Platform](https://img.shields.io/badge/Platform-STM32-blue.svg)](https://www.st.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/Status-Under%20Development-yellow.svg)]()

**Saratech OP** is an embedded control library for STM32-based Open-Weight soccer robots.

It provides a hardware abstraction and motion-control interface for systems built around GTD35 BLDC motor controllers, TDAxis12 IMU hardware, analog sensors, an OLED display, and shooter actuators.

The main goal is to provide a reusable control layer between the robot's hardware and its application software.

## Overview

Saratech OP provides interfaces for:

- Four-motor GTD35 control.
- Physical-unit motion commands.
- Omnidirectional movement.
- Coordinate-based movement commands.
- Motion acceleration limits.
- Wheel geometry configuration.
- IMU communication and yaw control.
- TXAXIS compensation.
- TDAXIS calibration.
- Analog sensor acquisition.
- Four-direction LDR acquisition.
- Shooter servo control.
- Spinback motor control.
- OLED display access.

The library is designed for hardware control and motion execution.

Robot strategy, computer vision, ball detection, and application-specific decision-making belong outside the library.

## Design Architecture

Saratech OP uses a layered design.

```text
+---------------------------------------+
|           Robot Application           |
|                                       |
|  Strategy / Vision / Decision Making  |
|  OpenMV Integration / Game Logic      |
+---------------------------------------+
                    |
                    v
+---------------------------------------+
|           Saratech OP                 |
|                                       |
|  Motion Control                       |
|  Motor Management                     |
|  Sensor Management                    |
|  IMU Interface                        |
|  Shooter Control                      |
|  Display Interface                    |
+---------------------------------------+
                    |
                    v
+---------------------------------------+
|             Hardware                  |
|                                       |
|  STM32F103C8T6                        |
|  GTD35 Motor Controllers              |
|  TDAxis12                             |
|  Analog Sensors                       |
|  OLED SH1106                          |
|  Servo and Spinback Motor             |
+---------------------------------------+
```

This separation allows the application to change without requiring changes to the hardware-control layer.

## Target Hardware

| Component | Description |
|---|---|
| Main MCU | STM32F103C8T6 |
| Development framework | Arduino STM32 |
| Motor controller | GTD35 |
| Motor type | BLDC |
| Motor count | Up to four on the documented controller bus |
| IMU | TDAxis12 |
| Display | SH1106 OLED |
| Optical sensors | Four-direction LDR |
| Analog sensors | Bottom, left and right inputs |
| Shooter | Servo and trigger output |
| Spinback | PWM-controlled motor |

The exact capabilities depend on the installed hardware and the controller configuration.

## Hardware Pinout

The following table documents the current repository configuration.

### GTD35 Communication

| Function | STM32 Pin |
|---|---|
| UART TX | PA9 |
| UART RX | PA10 |

### IMU and OLED

| Function | STM32 Pin |
|---|---|
| I2C SCL | PB10 |
| I2C SDA | PB11 |
| OLED controller | SH1106 |
| OLED I2C address | Verify in hardware configuration |

The IMU and OLED use the documented I2C2 bus. Both devices must use compatible bus wiring and distinct I2C addresses.

### Analog Sensors

| Function | STM32 Pin |
|---|---|
| Bottom sensor | PA1 |
| Right sensor | PA2 |
| Left sensor | PA3 |

### LDR Sensors

| Function | STM32 Pin |
|---|---|
| Front LDR | PA5 |
| Right LDR | PA6 |
| Rear LDR | PA7 |
| Left LDR | PB0 |

### Shooter and Spinback

| Function | STM32 Pin |
|---|---|
| Kicker detection | PA4 |
| Shooter trigger | PC13 |
| Shooter servo | PA15 |
| Spinback motor | PB5 |

**Hardware note:** Verify alternate-function configuration, timer availability, JTAG/SWD configuration, and electrical connections before using this pinout on a different STM32 board revision.

## Installation

### Method 1: Arduino IDE

1. Download or clone this repository.
2. Extract the repository.
3. Copy the `saratech_op` directory into:

```text
Documents/Arduino/libraries/
```

4. Restart Arduino IDE.
5. Open one of the examples.

### Method 2: ZIP Installation

1. Download the repository as a ZIP archive.
2. Open Arduino IDE.
3. Select:

```text
Sketch
  → Include Library
  → Add .ZIP Library
```

4. Select the downloaded archive.

## Dependencies

The repository includes the required library sources inside `src/`.

| Library | Purpose |
|---|---|
| GTD35_Custom | GTD35 motor communication |
| TDAxis12 | IMU communication |
| STM32_ISR_Servo | Servo control |
| Adafruit_GFX | Graphics functions |
| Adafruit_SH1106_STM32 | SH1106 OLED support |

The repository includes these dependencies to simplify installation.

Their original source files should remain unchanged unless a separate, documented compatibility fix is required.

Refer to the individual dependency licenses before redistribution.

## Quick Start

```cpp
#include <saratech_op.h>

void setup() {
    saratech.begin();

    saratech.setWheelDiameter(5.0);
    saratech.setTDAxis(200);
    saratech.setLDRThreshold(300);
}

void loop() {
    saratech.updateSensors();

    if (saratech.getLDRFront()) {
        saratech.move(80.0, 40.0, 0.0);
    }
    else if (saratech.getLDRRight()) {
        saratech.move(80.0, 40.0, 90.0);
    }
    else if (saratech.getLDRLeft()) {
        saratech.move(80.0, 40.0, -90.0);
    }
    else {
        saratech.stopMotors();
    }

    saratech.updateTXAXISCompensation();
}
```

This example demonstrates the basic initialization, sensor acquisition, motion command, and orientation compensation flow.

## Motion Control

Saratech OP provides motion commands that use physical units instead of requiring the application to calculate raw motor outputs.

### Physical-Unit Movement

```cpp
saratech.move(speed_cm_s, accel_cm_s2, direction_deg);
```

Parameters:

| Parameter | Unit | Description |
|---|---|---|
| speed_cm_s | cm/s | Requested linear speed |
| accel_cm_s2 | cm/s² | Requested acceleration |
| direction_deg | degrees | Movement direction |

### Coordinate-Based Movement

```cpp
bool reached = saratech.goTo(
    targetX_cm,
    targetY_cm,
    speed_cm_s
);
```

This function provides a coordinate-based movement interface.

The application must provide a suitable position reference. A yaw-only IMU does not independently provide absolute X/Y position.

### Curved Approach

```cpp
float angle = saratech.shift(
    robotX,
    robotY,
    ballX,
    ballY,
    shiftCoeff
);
```

This function provides a geometric movement-direction calculation.

The robot position, target position, and coefficient must be supplied by the application.

The function does not perform computer vision or detect a ball.

## Wheel Geometry

The library supports configurable wheel geometry.

```cpp
saratech.setWheelDiameter(5.0);
saratech.setWheelRadius(2.5);
saratech.setWheelBase(16.0);
saratech.setTrackWidth(16.0);
```

Current documented geometry:

| Parameter | Value | Status |
|---|---:|---|
| Wheel diameter | 5.0 cm | Confirmed |
| Wheel radius | 2.5 cm | Calculated |
| Wheel circumference | 15.708 cm | Calculated |
| Wheelbase | 16.0 cm | Approximate |
| Track width | 16.0 cm | Approximate |

Wheelbase and track width must be measured on the actual robot before accurate motion calibration.

## Motion Limits

```cpp
saratech.setMaxSpeed(200.0);
saratech.setMaxAccel(100.0);
saratech.setMaxMotorRPM(1000.0);
```

| Parameter | Unit | Description |
|---|---|---|
| Max speed | cm/s | Maximum requested linear speed |
| Max acceleration | cm/s² | Maximum requested acceleration |
| Max motor RPM | RPM | Motor rotational speed limit |

These values are configuration examples, not guaranteed safe operating limits.

Set them according to the actual motor, controller, battery, and mechanical system.

## IMU and Orientation

The library provides an interface for TDAxis12 communication and yaw-related operations.

```cpp
saratech.setTDAxis(200);

saratech.setTXAxisGain(1.0f);

saratech.setTXAxisAngle(4000);

saratech.updateTXAXISCompensation();

saratech.rotateYaw(5.0);

saratech.resetYaw();
```

### TDAXIS

```cpp
saratech.setTDAxis(coefficient);
```

Documented coefficient range: 0–512.

### TXAXIS Compensation

```cpp
saratech.setTXAxisGain(1.0f);

saratech.setTXAxisAngle(4000);

saratech.updateTXAXISCompensation();
```

These functions configure and apply the documented compensation interface.

The exact mathematical behavior and calibration requirements must be checked against the implementation.

### Yaw Control

```cpp
saratech.rotateYaw(angleDeg);

saratech.resetYaw();
```

These functions provide yaw-related control and reference management.

## Sensor Management

### Update All Sensors

```cpp
saratech.updateSensors();
```

### Update Individual Sensor Groups

```cpp
saratech.updateIMU();

saratech.updateLDR();

saratech.updateAnalogSensors();
```

### Read LDR States

```cpp
bool front = saratech.getLDRFront();
bool right = saratech.getLDRRight();
bool back  = saratech.getLDRBack();
bool left  = saratech.getLDRLeft();
```

### Configure LDR Sensors

```cpp
saratech.setLDRThreshold(300);

saratech.setLDRBaseline(idx, value);
```

The threshold and baseline values must be calibrated for the actual sensors and environment.

## Direct Motor Control

The library also provides direct motor commands.

```cpp
saratech.setMotorSpeeds(fl, fr, rr, rl);

saratech.setMotorSpeedsRPM(
    fl_rpm,
    fr_rpm,
    rr_rpm,
    rl_rpm
);

saratech.stopMotors();
```

The direct motor interface accepts signed motor commands.

Check the implementation for the exact value range, sign convention, and RPM conversion behavior.

## Shooter Control

The library provides interfaces for shooter actuator control.

### Shooter Command

```cpp
saratech.shoot(servoAngle_deg, spinBackSpeed);
```

### Direct Servo Control

```cpp
saratech.setShooterServoAngle(deg);
```

### Spinback Control

```cpp
saratech.setSpinbackSpeed(pwm);
```

The documented spinback range is 0–255.

Servo limits must match the installed servo and mechanical assembly.

## Hardware Access

The library exposes access to selected hardware objects.

```cpp
saratech.gtd35();

saratech.imu();

saratech.servoCtrl();

saratech.display();
```

Use these interfaces when the application requires access beyond the high-level API.

Direct hardware access can bypass library-level control and configuration.

## Coordinate System

The documented robot coordinate system is:

| Axis or Angle | Direction |
|---|---|
| Positive X | Robot right |
| Positive Y | Robot forward |
| 0° | Forward |
| 90° | Right |
| -90° | Left |

### Units

| Quantity | Unit |
|---|---|
| Position | cm |
| Linear speed | cm/s |
| Acceleration | cm/s² |
| Angle | degrees |
| Motor speed | RPM |
| Spinback command | Integer, 0–255 |

## Examples

The repository provides two main examples.

### Basic_Control

A starting example for basic robot initialization and control.

### Complete_Reference

The main API reference example.

It demonstrates the intended library interface and should be used as the primary reference when integrating Saratech OP into an application.

When the public API changes, update this example with the same changes.

## External Computer Vision

Saratech OP does not implement computer vision or ball detection.

An external vision device, such as OpenMV H7 Plus, can be integrated at the application level.

The application is responsible for:

- Receiving vision data.
- Processing object coordinates.
- Tracking objects.
- Calculating target positions.
- Selecting movement commands.
- Deciding when to shoot.

Saratech OP provides the hardware-control interface that the application can use to execute these decisions.

This separation prevents vision-specific code from becoming a dependency of the motion-control library.

## Verification and Testing

The repository contains a test report and a list of verification areas.

The following tests should be completed on the target hardware:

- STM32 compilation.
- GTD35 communication.
- Four-motor operation.
- IMU communication.
- LDR acquisition.
- Analog sensor acquisition.
- OLED communication.
- Shooter servo operation.
- Spinback control.
- Motion calculations.
- TDAXIS calibration.
- TXAXIS compensation.
- Coordinate-based movement.

A listed test is not proof of successful hardware verification.

Record the actual test conditions and results in `TEST_REPORT.md`.

## Repository Structure

```text
saratech_op/
│
├── src/
│   ├── saratech_op.h
│   ├── saratech_op.cpp
│   │
│   ├── GTD35_Custom/
│   ├── TDAxis12/
│   ├── STM32_ISR_Servo/
│   ├── Adafruit_GFX/
│   └── Adafruit_SH1106_STM32/
│
├── examples/
│   ├── Basic_Control/
│   │   └── Basic_Control.ino
│   │
│   └── Complete_Reference/
│       └── Complete_Reference.ino
│
├── Pinout.txt
├── TEST_REPORT.md
├── library.properties
└── README.md
```

Refer to the repository for the current directory contents.

## Development Guidelines

When contributing to Saratech OP:

1. Preserve the separation between hardware control and application logic.
2. Keep the GTD35 and TDAxis12 original drivers unchanged unless a documented fix is required.
3. Keep external computer vision outside the library.
4. Preserve consistent physical units.
5. Document all public API changes.
6. Update `Complete_Reference` when the API changes.
7. Verify compilation before submitting changes.
8. Record hardware test results.
9. Do not claim hardware compatibility without verification.

## Known Limitations

- Hardware behavior must be verified on the actual robot.
- Wheelbase and track width require mechanical measurement.
- Yaw data alone does not provide absolute robot position.
- Coordinate-based movement requires an appropriate position reference.
- Timer and alternate-function assignments must be checked for the target STM32 board.
- The library does not implement ball detection or computer vision.

## License

Check the repository and bundled dependency licenses before redistribution.

No additional license terms are asserted by this README.

## Author

**Saratech**

Repository: [saratech-OP](https://github.com/amirrezasadeghi0704/saratech-OP)

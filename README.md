# saratech_op

کتابخانه کنترل ربات فوتبالی Open-Weight دانشجو — لایه انتزاع سخت‌افزار برای STM32

Open-Weight Student Soccer Robot Control Library — Hardware Abstraction Layer for STM32

## نصب / Installation

پوشه `saratech_op` را در `Arduino/libraries/` قرار دهید.

Place the `saratech_op` folder into `Arduino/libraries/`.

## ساختار فایل‌ها / File Structure

```
saratech_op/
│
├── src/
│   ├── saratech_op.h          # سربرگ اصلی / Main header
│   ├── saratech_op.cpp        # پیاده‌سازی اصلی / Main implementation
│   │
│   ├── GTD35_Custom/          # GTD35 motor driver library ( untouched )
│   │   ├── GTD35.h
│   │   └── GTD35.cpp
│   │
│   ├── TDAxis12/              # IMU library ( untouched )
│   │   ├── TDAxis12.h
│   │   └── TDAxis12.cpp
│   │
│   └── STM32_ISR_Servo/       # Servo library ( untouched )
│       ├── STM32_ISR_Servo.h
│       ├── STM32_ISR_Servo.hpp
│       ├── STM32_ISR_Servo_Impl.h
│       ├── STM32_ISR_Servo_Debug.h
│       └── STM32_FastTimerInterrupt.h
│
├── examples/
│   ├── Basic_Control/         # Example 1: basic robot control
│   │   └── Basic_Control.ino
│   └── Complete_Reference/    # Example 2: full API reference
│       └── Complete_Reference.ino
│
├── Pinout.txt                 # Complete pinout reference
├── README.md                  # This file
└── TEST_REPORT.md             # Verification report
```

## وابستگی‌ها / Dependencies

تمام کتابخانه‌های ضروری داخل پوشه `src/` قرار دارند و نیازی به نصب جداگانه نیست.

All required libraries are included inside `src/` — no separate installation needed.

| کتابخانه / Library | منبع / Source | نسخه / Version |
|---|---|---|
| GTD35_Custom | `src/GTD35_Custom/` | 1.0.0 (original) |
| TDAxis12 | `src/TDAxis12/` | — (original) |
| STM32_ISR_Servo | `src/STM32_ISR_Servo/` | 1.1.0 (Khoi Hoang, MIT) |
| Adafruit_GFX | `src/Adafruit_GFX/` | — (original) |
| Adafruit_SH1106_STM32 | `src/Adafruit_SH1106_STM32/` | — (original) |

## پین‌های سخت‌افزار / Hardware Pins

| عملکرد / Function | پین / Pin | نوع / Type |
|---|---|---|
| GTD35 UART TX | PA9 | OUTPUT |
| GTD35 UART RX | PA10 | INPUT |
| IMU I2C SCL | PB10 | OUTPUT |
| IMU I2C SDA | PB11 | INPUT |
| Bottom sensor | PA1 | INPUT ANALOG |
| Right sensor | PA2 | INPUT ANALOG |
| Left sensor | PA3 | INPUT ANALOG |
| Front LDR | PA5 | INPUT ANALOG |
| Right LDR | PA6 | INPUT ANALOG |
| Back LDR | PA7 | INPUT ANALOG |
| Left LDR | PB0 | INPUT ANALOG |
| Kicker detect | PA4 | INPUT (active-LOW) |
| Shooter trigger | PC13 | OUTPUT (active-HIGH) |
| Shooter servo | PA15 | PWM (ISR) |
| Spinback motor | PB5 | PWM |
| OLED SH1106 SDA | PB11 | I2C2 |
| OLED SH1106 SCL | PB10 | I2C2 |

## هندسه چرخ‌ها / Wheel Geometry

| پارامتر / Parameter | مقدار / Value | واحد / Unit | وضعیت / Status |
|---|---|---|---|
| قطر چرخ / Wheel diameter | 5.0 | cm | ✅ تأیید شده / confirmed |
| شعاع چرخ / Wheel radius | 2.5 | cm | ✅ محاسبه شده / calculated |
| محیط چرخ / Wheel circumference | 15.708 | cm | ✅ محاسبه شده / calculated |
| Wheelbase (جلو-عقب) / فاصله محور | 16.0 | cm | ⚠️ تقریبی / approximate |
| Track width (چپ-راست) / فاصله محور | 16.0 | cm | ⚠️ تقریبی / approximate |

> ⚠️ Wheelbase و track width نیاز به اندازه‌گیری مکانیکی دارند.
> Wheelbase and track width require mechanical measurement.

## راه‌اندازی سریع / Quick Start

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
    } else if (saratech.getLDRRight()) {
        saratech.move(80.0, 40.0, 90.0);
    } else if (saratech.getLDRLeft()) {
        saratech.move(80.0, 40.0, -90.0);
    } else {
        saratech.stopMotors();
    }

    saratech.updateTXAXISCompensation();
}
```

## مرجع API / API Reference

### راه‌اندازی / Initialization
```cpp
saratech.begin();
saratech.begin(motorCfg, sensorCfg, shooterCfg, motionCfg);
```

### هندسه چرخ‌ها / Wheel Geometry
```cpp
saratech.setWheelDiameter(5.0);   // قطر چرخ (cm) / Wheel diameter (cm)
saratech.setWheelRadius(2.5);     // شعاع چرخ (cm) / Wheel radius (cm)
saratech.setWheelBase(16.0);      // فاصله محور جلو-عقب (cm) / Front-rear wheelbase
saratech.setTrackWidth(16.0);     // فاصله محور چپ-راست (cm) / Left-right track width
```

### محدودیت‌های حرکت / Motion Limits
```cpp
saratech.setMaxSpeed(200.0);      // cm/s / سرعت حداکثری / Max speed
saratech.setMaxAccel(100.0);      // cm/s² / شتاب حداکثری / Max acceleration
saratech.setMaxMotorRPM(1000.0);  // RPM / دور حداکثری موتور / Max motor RPM
```

### TDAXIS / TXAXIS
```cpp
saratech.setTDAxis(200);                      // ضریb TDAXIS (0-512) / TDAXIS coeff (0-512)
saratech.setTXAxisGain(1.0f);                 // ضریb تقویت / Gain
saratech.setTXAxisAngle(4000);                // زاویه پیش‌فرض / Default angle
saratech.updateTXAXISCompensation();          // jبران‌سازی / Compensation
saratech.rotateYaw(5.0);                      // چرخش عمودی / Yaw rotation
saratech.resetYaw();                          // بازیابی مرجع / Reset reference
```

### توابع پنج‌گانه اصلی / Five Main Functions
```cpp
// 1. حرکت با واحدهای فیزیکی / Move with physical units
saratech.move(speed_cm_s, accel_cm_s2, direction_deg);

// 2. حرکت مختصاتی / Coordinate-based movement
bool reached = saratech.goTo(targetX_cm, targetY_cm, speed_cm_s);

// 3. جبران‌سازی TXAXIS / TXAXIS compensation
saratech.updateTXAXISCompensation();

// 4A. کالیبراسیون TDAXIS / TDAXIS calibration
saratech.setTDAxis(coefficient);  // 0-512

// 4B. چرخش عمودی / Yaw rotation
saratech.rotateYaw(angleDeg);
saratech.resetYaw();

// 4C. شوتر / Shooter
saratech.shoot(servoAngle_deg, spinBackSpeed);

// 5. روش منحنی / Curved approach
float angle = saratech.shift(robotX, robotY, ballX, ballY, shiftCoeff);
```

### سنسورها / Sensors
```cpp
saratech.updateSensors();       // همه / All
saratech.updateIMU();           // ژیروسکوپ / Gyroscope
saratech.updateLDR();           // سنسورهای نوری / Photodiodes
saratech.updateAnalogSensors(); // سنسورهای آنالوگ / Analog sensors

bool front = saratech.getLDRFront();
bool right = saratech.getLDRRight();
bool back  = saratech.getLDRBack();
bool left  = saratech.getLDRLeft();
```

### تنظیمات سنسور / Sensor Config
```cpp
saratech.setLDRThreshold(300);
saratech.setLDRBaseline(idx, value);
```

### کنترل مستقیم / Direct Control
```cpp
saratech.setMotorSpeeds(fl, fr, rr, rl);          // signed int16
saratech.setMotorSpeedsRPM(fl_rpm, fr_rpm, rr_rpm, rl_rpm);
saratech.stopMotors();
saratech.setShooterServoAngle(deg);
saratech.setSpinbackSpeed(pwm);
```

### دسترسی به سخت‌افزار / Hardware Access
```cpp
saratech.gtd35();       // GTD35 object
saratech.imu();         // TDAxis12 object
saratech.servoCtrl();   // STM32_ISR_Servo object
saratech.display();     // Adafruit_SH1106 object (OLED)
```

## سیستم مختصات / Coordinate System

- **محور X مثبت**: راست ربات / Robot right
- **محور Y مثبت**: جلوی ربات / Robot forward
- **زاویه 0°**: به سمت جلو / Forward
- **زاویه 90°**: به سمت راست / Right
- **زاویه -90°**: به سمت چپ / Left
- **واحدها**: cm, cm/s, cm/s², degree

## واحد‌ها / Units

| مقدار / Quantity | واحد / Unit | محدوده / Range |
|---|---|---|
| سرعت خطی / Linear speed | cm/s | ±maxSpeed |
| شتاب / Acceleration | cm/s² | 0 to maxAccel |
| زاویه / Angle | degree | -180 to 180 |
| موقعیت / Position | cm | آزاد / Any |
| ضریb TDAXIS / TDAXIS coeff | integer | 0 to 512 |
| اسپین‌بک / Spinback speed | integer | 0 to 255 |
| زاویه سروو / Servo angle | degree | servoMin to servoMax |
| قطر چرخ / Wheel diameter | cm | 5.0 (confirmed) |

## محدودیت‌های سخت‌افزاری / Hardware Limitations

- GTD35: حداکثر 4 موتور روی یک bus
- IMU: فقط خواندن زاویه Yaw
- OpenMV H7 Plus: ارتباط با توپ‌یابی (در حال حاضر پیاده‌سازی نشده)

## مثال‌ها / Examples

- `Basic_Control` — مثال اولیه کنترل ربات
- `Complete_Reference` — مرجع کامل تمام APIها

## تست / Testing

- [x] ساختار فایل‌ها / File structure
- [x] کامپایل headfile / Header compilation
- [ ] تست GTD35 با موتورها / Motor test
- [ ] تست IMU / IMU test
- [ ] تست LDR / LDR test
- [ ] تست شوتر / Shooter test
- [ ] تست حرکت / Movement test
- [ ] تست TDAXIS/TXAXIS / Compensation test

## منبع کتابخانه‌ها / Library Sources

| کتابخانه / Library | منبع / Source | مجوز / License |
|---|---|---|
| GTD35_Custom | پروژه / Project | — |
| TDAxis12 | پروژه / Project | — |
| STM32_ISR_Servo | khoih-prog (GitHub) | MIT |

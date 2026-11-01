// =============================================
// Example 1: Basic Robot Control
// مثال ۱: کنترل پایه ربات
// =============================================

#include <saratech_op.h>

void setup() {
    // 1. راه‌اندازی اصلی (شامل OLED SH1106)
    // 1. Main initialization (includes OLED SH1106)
    saratech.begin();

    // 2. تنظیم مختصات چرخ‌ها (قطر 5 سانتی‌متر تأیید شده)
    // 2. Set wheel geometry (5cm diameter confirmed)
    saratech.setWheelDiameter(5.0);        // قطر چرخ = 5cm / Wheel diameter = 5cm
    saratech.setWheelBase(16.0);           // فاصله محور جلو-عقب / Front-rear wheelbase
    saratech.setTrackWidth(16.0);          // فاصله محور چپ-راست / Left-right track width

    // 3. تنظیم TDAXIS (ضریب جبران ژیروسکوپ)
    // 3. Set TDAXIS coefficient (gyro compensation scale)
    saratech.setTDAxis(200);               // مقدار تنظیم‌شده / Set value

    // 4. تنظیم حداکثر سرعت
    // 4. Set max speed
    saratech.setMaxSpeed(200.0);           // 200 cm/s
    saratech.setMaxAccel(100.0);           // 100 cm/s²

    // 5. تنظیم آستانه LDR
    // 5. Set LDR threshold
    saratech.setLDRThreshold(300);
}

void loop() {
    // 1. بروزرسانی سنسورها
    // 1. Update sensors
    saratech.updateSensors();

    // 2. حرکت بر اساس سنسورهای LDR (جلو/راست/چپ/عقب)
    // 2. Move based on LDR sensors (front/right/left/back)
    if (saratech.getLDRFront()) {
        saratech.move(80.0, 40.0, 0.0);    // حرکت به جلو / Move forward
    } else if (saratech.getLDRRight()) {
        saratech.move(80.0, 40.0, 90.0);   // حرکت به راست / Move right
    } else if (saratech.getLDRLeft()) {
        saratech.move(80.0, 40.0, -90.0);  // حرکت به چپ / Move left
    } else if (saratech.getLDRBack()) {
        saratech.move(80.0, 40.0, 180.0);  // حرکت به عقب / Move backward
    } else {
        saratech.stopMotors();             // توقف / Stop
    }

    // 3. به‌روزرسانی جبران‌سازی TXAXIS (نمایش هم روی Serial هم روی OLED)
    // 3. Update TXAXIS compensation (display on both Serial and OLED)
    saratech.updateTXAXISCompensation();
    saratech.printDebug();  // نمایش روی OLED و Serial
}

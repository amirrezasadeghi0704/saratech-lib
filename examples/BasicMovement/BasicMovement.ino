/**
 * @file BasicMovement.ino
 * @brief مثال کنترل حرکت پایه با sina_class
 * @version 2.0.0
 *
 * démontré:
 * - sina.motor() با ۳ ورودی اصلی + ۱ ورودی شتاب
 * - amirreza_class برای تنظیم ضریب TDaxis
 * - کنترل سرعت بر حسب سانتی‌متر بر ثانیه
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);

    // راه‌اندازی با GTD35 روی Serial1
    // پارامترها: I2C, آدرس TDAxis, Serial پورت, پین دایرکشن, سطح TX, Baud
    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);

    // تنظیم ضریب TDaxis برای همه توابع حرکتی (پیش‌فرض 200)
    amirreza.setTdaxisGain(200);

    Serial.println(F("=== Basic Movement Example ==="));
    Serial.println(F("Commands:"));
    Serial.println(F("  f <speed>  - Forward"));
    Serial.println(F("  b <speed>  - Backward"));
    Serial.println(F("  l <speed>  - Left"));
    Serial.println(F("  r <speed>  - Right"));
    Serial.println(F("  s          - Stop"));
    Serial.println(F("  a <gain>   - Set TDaxis gain"));
    Serial.println(F("==============================="));
}

void loop() {
    saratechUpdateSensors();  // ضروری: صدا زده شود هر loop

    handleSerialCommands();

    // مثال: حرکت خودکار برای تست
    static uint32_t lastAuto = 0;
    if (millis() - lastAuto > 5000) {
        static int dir = 0;
        float speeds[4] = {10, 15, 20, 25};
        float dirs[4][2] = {{0, 100}, {90, 0}, {0, -100}, {-90, 0}};

        sina.motor(speeds[dir], dirs[dir][0], dirs[dir][1], 200, 5);
        Serial.print(F("Auto move: "));
        Serial.println(dir);
        dir = (dir + 1) % 4;
        lastAuto = millis();
    }
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("f ")) {
        float speed = cmd.substring(2).toFloat();
        sina.motor(speed, 0, 100, 200, 5);  // Forward
        Serial.print(F("Forward: ")); Serial.println(speed);
    }
    else if (cmd.startsWith("b ")) {
        float speed = cmd.substring(2).toFloat();
        sina.motor(speed, 0, -100, 200, 5);  // Backward
        Serial.print(F("Backward: ")); Serial.println(speed);
    }
    else if (cmd.startsWith("l ")) {
        float speed = cmd.substring(2).toFloat();
        sina.motor(speed, -100, 0, 200, 5);  // Left
        Serial.print(F("Left: ")); Serial.println(speed);
    }
    else if (cmd.startsWith("r ")) {
        float speed = cmd.substring(2).toFloat();
        sina.motor(speed, 100, 0, 200, 5);  // Right
        Serial.print(F("Right: ")); Serial.println(speed);
    }
    else if (cmd == "s") {
        sina.stop();
        Serial.println(F("Stopped"));
    }
    else if (cmd.startsWith("a ")) {
        float gain = cmd.substring(2).toFloat();
        amirreza.setTdaxisGain(gain);
        Serial.print(F("TDaxis Gain: ")); Serial.println(gain);
    }
    else if (cmd == "status") {
        RobotState& state = RobotState::instance();
        Serial.print(F("Yaw: ")); Serial.println(state.yaw);
        Serial.print(F("Pos: ")); Serial.print(state.posX); Serial.print(F(", ")); Serial.println(state.posY);
    }
}
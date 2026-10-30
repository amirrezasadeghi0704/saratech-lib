/**
 * @file ShootingAiming.ino
 * @brief مثال شوت و هدف‌گیری با Amirreza (کلاس اصلی)
 * @version 2.0.0
 *
 * demonstré:
 * - amirreza.clapm() - چرخش به سمت توپ
 * - amirreza.clapmshoot() - هدف‌گیری دروازه و شوت
 * - amirreza.shootclamp() - سروو با یاو ثابت
 * - amirreza.smshoot() - شوت هوشمند ترکیبی
 * - amirreza.setTdaxisGain() - تنظیم ضریب TDaxis برای همه حرکات
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);

    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);

    amirreza.setTdaxisGain(200);

    // اگر شوتر داشته باشید، پین‌ها را تنظیم کنید
    // amirreza.shoot(PC13, PA4);  // پین فرمان، پین فیدبک

    Serial.println(F("=== Shooting & Aiming Example ==="));
    Serial.println(F("Commands:"));
    Serial.println(F("  c <maxRot> <rx> <ry> <bx> <by>  - clapm: Face ball"));
    Serial.println(F("  g <rx> <ry> <gx> <gy>           - clapmshoot: Face goal & shoot"));
    Serial.println(F("  s                                  - shootclamp: Servo aim"));
    Serial.println(F("  m <speed> <n> <i> <z>             - smshoot: Smart shoot"));
    Serial.println(F("  t <gain>                          - Set TDaxis gain"));
    Serial.println(F("  b                                  - Check ball ready"));
    Serial.println(F("  status                            - Print status"));
    Serial.println(F("============================================="));
}

void loop() {
    saratechUpdateSensors();

    handleSerialCommands();

    // اگر در حالت smshoot هستیم، باید مرتب صدا زده شود
    // amirreza.smshoot(...) در کلاس اصلی Amirreza وجود دارد
    // این مثال از کلاس‌های اصلی استفاده می‌کند (Amirreza.h)
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("c ")) {
        // clapm: maxRot robotX robotY ballX ballY
        float maxRot, rx, ry, bx, by;
        int parsed = sscanf(cmd.c_str(), "c %f %f %f %f %f", &maxRot, &rx, &ry, &bx, &by);
        if (parsed == 5) {
            // Note: Using original Amirreza class from Amirreza.h
            // This requires the global 'amirreza' instance from Amirreza.h
            // which is different from amirreza_class
            Serial.print(F("clapm: maxRot=")); Serial.print(maxRot);
            Serial.print(F(" robot=(")); Serial.print(rx); Serial.print(F(",")); Serial.print(ry);
            Serial.print(F(") ball=(")); Serial.print(bx); Serial.print(F(",")).print(by).println(F(")"));
            // amirreza.clapm(maxRot, rx, ry, bx, by);  // Needs Amirreza instance
        }
    }
    else if (cmd.startsWith("g ")) {
        // clapmshoot: robotX robotY goalX goalY
        float rx, ry, gx, gy;
        int parsed = sscanf(cmd.c_str(), "g %f %f %f %f", &rx, &ry, &gx, &gy);
        if (parsed == 4) {
            Serial.print(F("clapmshoot: robot=(")); Serial.print(rx); Serial.print(F(",")).print(ry);
            Serial.print(F(") goal=(")).print(gx).print(F(",")).println(gy);
            // amirreza.clapmshoot(rx, ry, gx, gy);
        }
    }
    else if (cmd == "s") {
        // shootclamp
        Serial.println(F("shootclamp: Servo aim with fixed yaw"));
        // amirreza.shootclamp();
    }
    else if (cmd.startsWith("m ")) {
        // smshoot: speed n i z
        int speed, n, i, z;
        int parsed = sscanf(cmd.c_str(), "m %d %d %d %d", &speed, &n, &i, &z);
        if (parsed == 4) {
            Serial.print(F("smshoot: speed=")); Serial.print(speed);
            Serial.print(F(" n=")).print(n).print(F(" i=")).print(i).print(F(" z=")).println(z);
            // amirreza.smshoot(speed, n, i, z);
        }
    }
    else if (cmd.startsWith("t ")) {
        float gain = cmd.substring(2).toFloat();
        amirreza.setTdaxisGain(gain);
        Serial.print(F("TDaxis Gain: ")); Serial.println(gain);
    }
    else if (cmd == "b") {
        Serial.print(F("Ball ready: "));
        // Serial.println(amirreza.checkBallReady() ? F("YES") : F("NO"));
    }
    else if (cmd == "status") {
        RobotState& state = RobotState::instance();
        Serial.println(F("\n--- Shooting Status ---"));
        Serial.print(F("Ball: ")); Serial.print(state.ballAngle); Serial.print(F(" deg, "));
        Serial.print(state.ballDistance); Serial.print(F(" units, "));
        Serial.println(state.ballValid ? F("VALID") : F("INVALID"));
        Serial.print(F("In Kicker: ")); Serial.println(state.ballInKicker ? F("YES") : F("NO"));
        Serial.print(F("Shooter Ready: ")); Serial.println(state.shooterReady ? F("YES") : F("NO"));
        Serial.print(F("Shooter Firing: ")); Serial.println(state.shooterFiring ? F("YES") : F("NO"));
        Serial.print(F("Servo: ")); Serial.print(state.servoAngle); Serial.println(F(" deg"));
        Serial.print(F("TDaxis Gain: ")); Serial.println(amirreza.getTdaxisGain());
    }
    else if (cmd == "help" || cmd == "?") {
        Serial.println(F("\nCommands:"));
        Serial.println(F("  c <maxRot> <rx> <ry> <bx> <by>  - clapm: Face ball"));
        Serial.println(F("  g <rx> <ry> <gx> <gy>           - clapmshoot: Face goal & shoot"));
        Serial.println(F("  s                                  - shootclamp: Servo aim + fixed yaw"));
        Serial.println(F("  m <speed> <n> <i> <z>             - smshoot: Smart combined shoot"));
        Serial.println(F("  t <gain>                          - Set TDaxis gain (applies to all)"));
        Serial.println(F("  b                                  - Check ball ready (PA4)"));
        Serial.println(F("  status                            - Print status"));
    }
}
/**
 * @file RotationServo.ino
 * @brief مثال چرخش و کنترل سروو با sina_rotation_class
 * @version 2.0.0
 *
 * demonstré:
 * - sina_rotation.rotate() با ۲ ورودی: اضافه کردن دستی به ژیرو + زاویه سروو
 * - کنترل چرخش ربات با اضافه کردن دستی به مقدار ژیروسکپ
 * - کنترل سروو تورتی (تورتی شوت)
 */

#include <saratech.h>

TwoWire i2c2(2);

void setup() {
    Serial.begin(115200);

    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);

    amirreza.setTdaxisGain(200);

    Serial.println(F("=== Rotation + Servo Example ==="));
    Serial.println(F("Commands:"));
    Serial.println(F("  r <gyro_add> <servo_angle>  - Rotate with manual gyro addition + servo"));
    Serial.println(F("  s                          - Stop rotation"));
    Serial.println(F("  v <angle>                   - Set servo angle only (45-135)"));
    Serial.println(F("  o <offset>                  - Servo offset from center (90°)"));
    Serial.println(F("  t <gain>                    - Set TDaxis gain"));
    Serial.println(F("  status                      - Print status"));
    Serial.println(F("=========================================="));
    Serial.println(F("Note: gyro_add is ADDED to current gyro before coefficient multiplication"));
    Serial.println(F("      servo_angle is absolute angle (45=left, 90=center, 135=right)"));
}

void loop() {
    saratechUpdateSensors();

    handleSerialCommands();
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("r ")) {
        // r <gyro_addition> <servo_angle>
        float gyroAdd, servoAng;
        int parsed = sscanf(cmd.c_str(), "r %f %f", &gyroAdd, &servoAng);
        if (parsed == 2) {
            sina_rotation.rotate(gyroAdd, servoAng);
            Serial.print(F("Rotate: gyro_add=")); Serial.print(gyroAdd);
            Serial.print(F(" deg, servo=")); Serial.print(servoAng); Serial.println(F(" deg"));
        }
    }
    else if (cmd == "s") {
        sina_rotation.stop();
        Serial.println(F("Rotation stopped"));
    }
    else if (cmd.startsWith("v ")) {
        float angle = cmd.substring(2).toFloat();
        if (angle >= 45 && angle <= 135) {
            servoAdapter.setAngle(angle);
            Serial.print(F("Servo set to: ")); Serial.println(angle);
        } else {
            Serial.println(F("Error: Servo angle must be 45-135"));
        }
    }
    else if (cmd.startsWith("o ")) {
        float offset = cmd.substring(2).toFloat();
        float target = 90.0f + offset;
        if (target >= 45 && target <= 135) {
            servoAdapter.setAngle(target);
            Serial.print(F("Servo offset: ")); Serial.print(offset);
            Serial.print(F(" -> angle: ")); Serial.println(target);
        } else {
            Serial.println(F("Error: Offset results in angle outside 45-135"));
        }
    }
    else if (cmd.startsWith("t ")) {
        float gain = cmd.substring(2).toFloat();
        amirreza.setTdaxisGain(gain);
        Serial.print(F("TDaxis Gain: ")); Serial.println(gain);
    }
    else if (cmd == "status") {
        RobotState& state = RobotState::instance();
        Serial.println(F("\n--- Rotation/Servo Status ---"));
        Serial.print(F("Yaw: ")); Serial.print(state.yaw);
        Serial.print(F(" deg, dgy: ")); Serial.print(state.dgy);
        Serial.print(F(" deg, Effective: ")); Serial.println(state.yaw + state.dgy);
        Serial.print(F("Servo: ")); Serial.print(state.servoAngle); Serial.println(F(" deg"));
        Serial.print(F("TDaxis Gain: ")); Serial.println(amirreza.getTdaxisGain());
    }
    else if (cmd == "help" || cmd == "?") {
        Serial.println(F("\nCommands:"));
        Serial.println(F("  r <gyro_add> <servo_angle>  - Rotate with manual gyro + servo"));
        Serial.println(F("  s                          - Stop rotation"));
        Serial.println(F("  v <45-135>                 - Set servo absolute angle"));
        Serial.println(F("  o <offset>                 - Servo offset from center (90°)"));
        Serial.println(F("  t <gain>                   - Set TDaxis gain"));
        Serial.println(F("  status                     - Print status"));
    }
}
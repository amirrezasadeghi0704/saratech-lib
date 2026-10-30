/**
 * @file FieldNavigation.ino
 * @brief مثال ناوبری میدانی با sadra_class و king.forati_class
 * @version 2.0.0
 *
 * demonstré:
 * - sadra.move() برای رفتن به مختصات (۳ ورودی)
 * - king.forati_class برای بازگشت به نقطه شروع (۳ ورودی)
 * - استفاده از/mapcalc برای به‌روزرسانی موقعیت
 */

#include <saratech.h>

TwoWire i2c2(2);

// نقاط پیش‌تعریف شده میدانی (سانتی‌متر)
struct Waypoint {
    const char* name;
    int x, y;
};

Waypoint waypoints[] = {
    {"Start/Origin", 0, 0},
    {"Center", 90, 120},
    {"Our Goal", 0, 0},
    {"Opp Goal", 90, 240},
    {"Left Corner", 0, 240},
    {"Right Corner", 180, 240},
    {"Penalty Spot", 90, 180},
    {"Midfield", 90, 120}
};
const int NUM_WAYPOINTS = sizeof(waypoints) / sizeof(waypoints[0]);

int currentWaypoint = 0;
bool autoMode = false;

void setup() {
    Serial.begin(115200);

    TwoWire i2c2(2);
    saratechBeginGTD35(&i2c2, 0x10, Serial1, -1, HIGH, 115200);

    amirreza.setTdaxisGain(200);

    Serial.println(F("=== Field Navigation Example ==="));
    Serial.println(F("Commands:"));
    Serial.println(F("  g <index>    - Go to waypoint (0-7)"));
    Serial.println(F("  a            - Toggle auto mode"));
    Serial.println(F("  r            - Return to origin (king.forati)"));
    Serial.println(F("  s            - Stop"));
    Serial.println(F("  p            - Print position"));
    Serial.println(F("  w            - List waypoints"));
    Serial.println(F("==============================="));
    listWaypoints();
}

void loop() {
    saratechUpdateSensors();

    handleSerialCommands();

    // Auto mode: move through waypoints sequentially
    if (autoMode) {
        if (!sadra.isMoving()) {
            currentWaypoint = (currentWaypoint + 1) % NUM_WAYPOINTS;
            Waypoint& wp = waypoints[currentWaypoint];
            sadra.move(wp.x, wp.y, 15);
            Serial.print(F("Auto -> ")); Serial.print(wp.name);
            Serial.print(F(" (")).print(wp.x).print(F(",")).print(wp.y).println(F(")"));
        }
    }

    // Sadra handles movement internally
    sadra.move(0, 0, 0);  // This just checks arrival in the library
}

void handleSerialCommands() {
    if (!Serial.available()) return;

    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("g ")) {
        int idx = cmd.substring(2).toInt();
        if (idx >= 0 && idx < NUM_WAYPOINTS) {
            currentWaypoint = idx;
            Waypoint& wp = waypoints[idx];
            sadra.move(wp.x, wp.y, 15);
            Serial.print(F("Going to ")); Serial.print(wp.name);
            Serial.print(F(" (")).print(wp.x).print(F(",")).print(wp.y).println(F(")"));
        }
    }
    else if (cmd == "a") {
        autoMode = !autoMode;
        if (autoMode) {
            currentWaypoint = 0;
            Waypoint& wp = waypoints[0];
            sadra.move(wp.x, wp.y, 15);
            Serial.println(F("Auto mode ON"));
        } else {
            sadra.stop();
            Serial.println(F("Auto mode OFF"));
        }
    }
    else if (cmd == "r") {
        autoMode = false;
        king.forati(15, 0, 0);  // Return to origin at 15 cm/s
        Serial.println(F("Returning to origin..."));
    }
    else if (cmd == "s") {
        autoMode = false;
        sadra.stop();
        Serial.println(F("Stopped"));
    }
    else if (cmd == "p") {
        printPosition();
    }
    else if (cmd == "w") {
        listWaypoints();
    }
    else if (cmd == "help" || cmd == "?") {
        Serial.println(F("\nCommands:"));
        Serial.println(F("  g <0-7>    - Go to waypoint"));
        Serial.println(F("  a          - Toggle auto mode"));
        Serial.println(F("  r          - Return to origin (king.forati)"));
        Serial.println(F("  s          - Stop"));
        Serial.println(F("  p          - Print position"));
        Serial.println(F("  w          - List waypoints"));
    }
}

void printPosition() {
    RobotState& state = RobotState::instance();
    Serial.println(F("\n--- Position ---"));
    Serial.print(F("Pos: ")); Serial.print(state.posX); Serial.print(F(", ")); Serial.println(state.posY);
    Serial.print(F("Yaw: ")); Serial.println(state.yaw);
    Serial.print(F("Distances: F=")); Serial.print(state.distFront);
    Serial.print(F(" R=")); Serial.print(state.distRight);
    Serial.print(F(" B=")); Serial.print(state.distBack);
    Serial.print(F(" L=")); Serial.println(state.distLeft);
    Serial.print(F("Ball: ")); Serial.print(state.ballAngle); Serial.print(F(" deg, "));
    Serial.print(state.ballDistance); Serial.println(F(" units"));
}

void listWaypoints() {
    Serial.println(F("\nWaypoints:"));
    for (int i = 0; i < NUM_WAYPOINTS; i++) {
        Serial.print(i); Serial.print(F(": "));
        Serial.print(waypoints[i].name);
        Serial.print(F(" (")).print(waypoints[i].x).print(F(",")).print(waypoints[i].y).println(F(")"));
    }
}
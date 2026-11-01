#ifndef GTD35_H
#define GTD35_H

#include <Arduino.h>

/*
 * GTD35 motor driver serial protocol.
 *
 * UART protocol:
 *   Header: 0xAA 0xAA
 *   Set ID: [0xAA, 0xAA, 0x00, previousID, 0x11, newID, XOR]
 *   Motion: [0xAA, 0xAA, motorCount, IDs..., per-motor data..., XOR]
 *
 * Motion data per motor:
 *   control, speed MSB, speed LSB, angle MSB, angle LSB
 *
 * The angle is transmitted as the raw 16-bit value. The examples use 4000,
 * matching the user's working four-motor example.
 */

class GTD35
{
public:
    explicit GTD35(HardwareSerial &serialPort);

    // Optional RS485 direction pin. txLevel is the pin level for transmit;
    // the opposite level is used for receive. Pass -1 if direction is automatic.
    void begin(uint32_t baud = 115200, int8_t directionPin = -1,
               uint8_t txLevel = HIGH);

    // Change the address of one connected motor.
    // Connect only one motor while assigning IDs.
    bool setMotorID(uint8_t previousID, uint8_t newID);
    void setID(uint8_t newID);

    // Configure the IDs addressed by motor(v0, v1, v2, v3).
    // Defaults to IDs 0, 1, 2, 3.
    void setMotorIDs(uint8_t id0, uint8_t id1, uint8_t id2, uint8_t id3);

    // Send four signed RPM/speed references in L1, L2, R2, R1 order.
    // Values are transmitted as signed 16-bit two's-complement values.
    bool motor(int16_t v0, int16_t v1, int16_t v2, int16_t v3);

    // Set the raw 16-bit angle value included in each motor's motion data.
    void setAngle(uint16_t angle);

    // Control the motor enable and encoder-reset bits.
    void setEnabled(bool enabled);
    void setResetEncoder(bool reset);

    // XOR checksum helper, useful for protocol testing.
    static uint8_t checksum(const uint8_t *data, size_t length);

private:
    HardwareSerial &_serial;
    int8_t _directionPin;
    uint8_t _txLevel;
    uint8_t _motorIDs[4];
    uint16_t _angle;
    bool _enabled;
    bool _resetEncoder;

    void sendFrame(const uint8_t *data, size_t length);
};

#endif

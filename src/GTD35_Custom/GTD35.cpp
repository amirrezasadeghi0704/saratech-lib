#include "GTD35.h"

GTD35::GTD35(HardwareSerial &serialPort)
  : _serial(serialPort),
    _directionPin(-1),
    _txLevel(HIGH),
    _angle(4000),
    _enabled(true),
    _resetEncoder(false) {
  _motorIDs[0] = 0;
  _motorIDs[1] = 1;
  _motorIDs[2] = 2;
  _motorIDs[3] = 3;
}

void GTD35::begin(uint32_t baud, int8_t directionPin, uint8_t txLevel) {
  _directionPin = directionPin;
  _txLevel = txLevel;

  if (_directionPin >= 0) {
    pinMode(_directionPin, OUTPUT);
    digitalWrite(_directionPin, !_txLevel); // Start in receive mode.
  }

  _serial.begin(baud);
}

uint8_t GTD35::checksum(const uint8_t *data, size_t length) {
  uint8_t result = 0;
  for (size_t i = 0; i < length; ++i) {
    result ^= data[i];
  }
  return result;
}

void GTD35::sendFrame(const uint8_t *data, size_t length) {
  if (_directionPin >= 0) {
    digitalWrite(_directionPin, _txLevel);
    delayMicroseconds(2);
  }

  _serial.write(data, length);
  _serial.flush(); // Wait for the final UART byte to finish transmitting.

  if (_directionPin >= 0) {
    delayMicroseconds(2);
    digitalWrite(_directionPin, !_txLevel);
  }
}

bool GTD35::setMotorID(uint8_t previousID, uint8_t newID) {
  uint8_t frame[7] = {
    0xAA,
    0xAA,
    0x00,
    previousID,
    0x11,
    newID,
    0x00
  };

  // XOR covers bytes 2 through 5 inclusive, matching the supplied code.
  frame[6] = checksum(&frame[2], 4);
  sendFrame(frame, sizeof(frame));
  return true; // The supplied protocol example does not show an ACK response.
}

void GTD35::setID(uint8_t newID) {
  for (uint16_t previousID = 0; previousID < 255; previousID++) {
    if (previousID == newID) continue;

    setMotorID((uint8_t)previousID, newID);
    delay(5);
  }
}

void GTD35::setMotorIDs(uint8_t id0, uint8_t id1,
                        uint8_t id2, uint8_t id3) {
  _motorIDs[0] = id0;
  _motorIDs[1] = id1;
  _motorIDs[2] = id2;
  _motorIDs[3] = id3;
}

void GTD35::setAngle(uint16_t angle) {
  _angle = angle;
}

void GTD35::setEnabled(bool enabled) {
  _enabled = enabled;
}

void GTD35::setResetEncoder(bool reset) {
  _resetEncoder = reset;
}

bool GTD35::motor(int16_t v0, int16_t v1, int16_t v2, int16_t v3) {
  const int16_t speeds[4] = {v0, v1, v2, v3};
  uint8_t frame[2 + 1 + 4 + (4 * 5) + 1];
  size_t pos = 0;

  frame[pos++] = 0xAA;
  frame[pos++] = 0xAA;
  frame[pos++] = 4;

  for (uint8_t i = 0; i < 4; ++i) {
    frame[pos++] = _motorIDs[i];
  }

  const uint8_t control =
      (_enabled ? 0x0F : 0x00) |
      (_resetEncoder ? 0xF0 : 0x00);

  for (uint8_t i = 0; i < 4; ++i) {
    const uint16_t speedBits = static_cast<uint16_t>(speeds[i]);

    frame[pos++] = control;
    frame[pos++] = static_cast<uint8_t>((speedBits >> 8) & 0xFF);
    frame[pos++] = static_cast<uint8_t>(speedBits & 0xFF);
    frame[pos++] = static_cast<uint8_t>((_angle >> 8) & 0xFF);
    frame[pos++] = static_cast<uint8_t>(_angle & 0xFF);
  }

  frame[pos] = checksum(&frame[2], pos - 2);
  ++pos;

  sendFrame(frame, pos);
  return true;
}

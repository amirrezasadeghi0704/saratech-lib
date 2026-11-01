#ifndef TDAxis12_H
#define TDAxis12_H

#include <Arduino.h>
#include <Wire.h>

class TDAxis12 {
public:
    TDAxis12(TwoWire *wire = &Wire, uint8_t address = 0x10);

    void begin(TwoWire *wire = &Wire, uint8_t address = 0x10);
    float read();

    float angle;           // last computed angle
    uint8_t sensors[16];   // raw sensor readings (0-255)
    bool is_ball = false;

private:
  uint8_t _addr;
  TwoWire *_wire;
  const uint8_t _count = 16;
};

#endif

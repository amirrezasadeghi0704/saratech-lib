#include "TDAxis12.h"
#include <math.h>

TDAxis12::TDAxis12(TwoWire *wire, uint8_t address) : _addr(address), _wire(wire) {
}

void TDAxis12::begin(TwoWire *wire, uint8_t address) {
  _wire = wire;
}

float TDAxis12::read() {
    _wire->beginTransmission(_addr);
    _wire->write(4);
    _wire->endTransmission();
    _wire->requestFrom(_addr, 18);
    uint8_t data[18];
    for(int i=0; i<18; i++) data[i] = _wire->read();
    int16_t z = (int16_t) data[0] << 8 | data[1];
    float angle = z / 100.0;
    return -angle;
}

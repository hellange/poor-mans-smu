#include "Arduino.h"
#include <SPI.h>
#include "SerialRAM.h"

class RamClass {

private:

  union flu {
    float val;
    uint8_t b[4];
  };
public:
  void init();
  float readRAMfloat(uint32_t address);
  void writeRAMfloat(uint32_t address, float value);
};

extern RamClass RAM;

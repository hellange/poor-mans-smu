#ifndef RAMCLASS_H
#define RAMCLASS_H


#include "Arduino.h"
#include <SPI.h>
#include "SerialRAM.h"



  union flu {
    float val;
    uint8_t b[4];
  };

  union tiu {
    uint32_t val;
    uint8_t b[4];
  };
  
  struct timedLog {
    flu value;
    tiu time;
  };
  
class RamClass {

private:

  int maxLogAddress = 300; //16000;
  int currentLogAddress;
  uint32_t logStartAddress = 0;
  void testReadWriteLogData();
  
public:
  boolean full = false;

  void init();
  float readRAMfloat(uint32_t address);
  void writeRAMfloat(uint32_t address, float value);

  timedLog readLogData(uint32_t logAddress);
  void writeLogData(uint32_t logAddress, float value, uint32_t time);

  void startLog();
  void logData(float value);

  int getCurrentLogAddress();
  int getMaxLogAddress();

  int nextAdr(float adr_);
};

extern RamClass RAM;

#endif

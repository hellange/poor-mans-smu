#include "RamClass.h"
#include "Arduino.h"



RamClass RAM;

void RamClass::init() {
   Serial.println("Initial RAM testing...");
   // just some initial read/write to check that device is probably there
   ram_init();
   int errors = 0;
   for (int i = 0; i < 10; i++) {
    uint8_t valueToWrite = i+3;
    ram_write(&valueToWrite, i, 1);
    uint8_t valueRead;
    ram_read(&valueRead, i, 1);
    //Serial.println(valueRead);
    //Serial.flush();
    if (valueRead != valueToWrite) {
      errors++;
    }
  }
  if (errors > 0) {
    Serial.println("ERROR: Initial RAM test failed !  No ram mounted ?");
    return;
  } else {
    Serial.println("Ram detected.");
  }

 // 1Mbit = 1024Mbit = 128Kbytes = 32000 floats...
 int maxBytes = 32000 * 4;
 Serial.print("Test full address space of ");  
 Serial.print(maxBytes);
 Serial.println(" bytes");
 
 Serial.println("Write all, then read all...");
 int address;
 float valueToWrite = 0;
  for (address = 0; address < maxBytes; address=address + 4) {
    writeRAMfloat(address, valueToWrite);
    valueToWrite += 0.01;
  }
  Serial.print("Wrote to ");
  Serial.print(maxBytes);
  Serial.println(" bytes.");
  valueToWrite = 0.0;
 for (address = 0; address < maxBytes; address=address + 4) {
    float valueRead = readRAMfloat(address);
    if (valueRead != valueToWrite) {
      Serial.print("ERROR in address ");
      Serial.println(address);
      Serial.print("Expected:");
      Serial.print(valueToWrite);
      Serial.println(", read:");
      Serial.println(valueRead);
      Serial.println("RAM test failed !");
      break;
    }
    valueToWrite += 0.01;
  }
  Serial.print("Read from ");
  Serial.print(maxBytes);
  Serial.println(" bytes.");
  Serial.println("RAM test success !");



}
float RamClass::readRAMfloat(uint32_t address) {
   flu valueRead;
   ram_read(&valueRead.b[0], address, 4);
   return valueRead.val;
}

void RamClass::writeRAMfloat(uint32_t address, float value) {
  flu valueToWrite;
  valueToWrite.val = value;
  ram_write(&valueToWrite.b[0], address, 4);
}

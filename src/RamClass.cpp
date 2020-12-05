#include "RamClass.h"
#include "Arduino.h"



RamClass RAM;
elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs

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
 // =128K/8 timed log data = 16K 
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
  testReadWriteLogData();
}

void RamClass::testReadWriteLogData() {
  Serial.println("===== TEST LOG DATA =====");
 int address;
 float valueToWrite = 0;
 uint32_t time;
  for (address = 0; address < 10; address=address + 1) {
    time = timeElapsed;
    Serial.print("Write to log address ");
    Serial.print(address);
    Serial.print(" data:");
    Serial.print(valueToWrite);
    Serial.print(", time:");
    Serial.println(time);
    writeLogData(address, valueToWrite, time);
    valueToWrite += 0.01;
    delay(20);
  }

  for (address = 0; address < 10; address=address + 1) {
    timedLog logData = readLogData(address);
    Serial.print("Read from log address ");
    Serial.print(address);
    Serial.print(" data:");
    Serial.print(logData.value.val);
    Serial.print(", time:");
    Serial.println(logData.time.val);
  }

  
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


// log data stored as 8 bytes: <4 byte time><4 byte floatvalue>
timedLog RamClass::readLogData(uint32_t address) {
  timedLog logData;
  ram_read(&logData.time.b[0], logStartAddress + address * 8, 4);
  //flu floatValue;
  logData.value.val = readRAMfloat(logStartAddress + address * 8 + 4);
  return logData;
}


void RamClass::writeLogData(uint32_t address, float value, uint32_t time) {
  tiu timeToWrite;
  timeToWrite.val = time;
  ram_write(&timeToWrite.b[0], address *8, 4);
  writeRAMfloat(logStartAddress + address * 8 + 4, value);
}

void RamClass::startLog() {
  timeElapsed = 0;
}

int oldestLogAddress;
void RamClass::logData(float value) {
  uint32_t t = timeElapsed;
  writeLogData(currentLogAddress, value, t);
  Serial.println("Logged ");
  Serial.print(value);
  Serial.print(" at log address ");
  Serial.print(currentLogAddress);
  Serial.print(" at time ");
  Serial.println(t);
  currentLogAddress ++;
  if (currentLogAddress > maxLogAddress) {
    full = true;
    currentLogAddress = 0;
  }
  if (full) {
    oldestLogAddress = currentLogAddress -1;
    if (oldestLogAddress < 0) {
      oldestLogAddress = maxLogAddress ;
    }
  } else {
    oldestLogAddress = 0;
  }
}

int RamClass::nextAdr(float adr_) {
  int adr = adr_;
  adr = adr -1;
  //Serial.print(adr);
  if (adr < 0 && full) {
    adr = maxLogAddress;
    //Serial.print("!!!!!!!!!!!!!!!  1 ");
    //Serial.print("Adr is");
   // Serial.print(adr);
  }
  if (adr < 0 && !full) {
    adr = -1; //currentLogAddress;
    //Serial.print("!!!!!!!!!!!!!!!  2");
  }
  return adr;
}



int RamClass::getCurrentLogAddress() {
  return currentLogAddress;
}
int RamClass::getMaxLogAddress() {
  return maxLogAddress;
}

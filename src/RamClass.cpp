#include "RamClass.h"
#include "Arduino.h"


// Set to false if you have external SPI ram mounted on your SMU.
// Set to true if you want to use smaller internal ram
boolean USE_NORMAL_RAM = true; 


float normalRamUndefinedValue = 999999.0;
float normalRamValue[1000];
uint32_t normalRamTime[1000];


RamClass RAM;
elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs

void RamClass::init() {
   Serial.println("Initial RAM testing...");
   if (USE_NORMAL_RAM) {
     for (int i=0;i<1000;i++) {
       normalRamValue[i] = 99000.0;//normalRamUndefinedValue;
     }
     return;
   }
  
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
    Serial.println("ERROR: Initial external SPI RAM test failed !  No SPI ram mounted ?");
    initFailure=true;
    return;
  } else {
    Serial.println("External SPI ram detected.");
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


bool RamClass::useNormalRam() {
  return USE_NORMAL_RAM;
}

// log data stored as 8 bytes: <4 byte time><4 byte floatvalue>
timedLog RamClass::readLogData(uint32_t address) {
  timedLog logData;
  if (USE_NORMAL_RAM) {
    logData.time.val = normalRamTime[address];
    logData.value.val = normalRamValue[address];
    return logData;
  } else {
    ram_read(&logData.time.b[0], logStartAddress + address * 8, 4);
    //flu floatValue;
    logData.value.val = readRAMfloat(logStartAddress + address * 8 + 4);
    return logData;
  }


  
}


void RamClass::writeLogData(uint32_t address, float value, uint32_t time) {
  if (USE_NORMAL_RAM) {
    normalRamValue[address] = value;
    normalRamTime[address] = time;
  } else {
    tiu timeToWrite;
    timeToWrite.val = time;
    ram_write(&timeToWrite.b[0], address *8, 4);
    writeRAMfloat(logStartAddress + address * 8 + 4, value);
  }
}

void RamClass::startLog() {
  timeElapsed = 0;
}


float logTotalSum = 0;
int nrOfLogEntriesForMean = 0;
void RamClass::logDataCalculateMean(float value, int nrOfValuesBeforeSave) {
  if (millis()< waitLogStart) {
    return;
  }
  logTotalSum = logTotalSum + value;
  nrOfLogEntriesForMean++;
  if (nrOfLogEntriesForMean >= nrOfValuesBeforeSave) {
     float val = logTotalSum/nrOfValuesBeforeSave;
     if (val>max) {
       max = val;
     }
     if (val < min) {
       min = val;
     }
     logData(val);
     nrOfLogEntriesForMean=0;
     logTotalSum = 0;
  }
}

int oldestLogAddress;
void RamClass::logData(float value) {
  uint32_t t = timeElapsed;

  // Serial.print("Trying to log ");
  // Serial.print(value,3);
  // Serial.print(" at log address ");
  // Serial.print(currentLogAddress);

  //Serial.println(t);

  unsigned long allSeconds=t/1000;
  int runHours= allSeconds/3600;
  int secsRemaining=allSeconds%3600;
  int runMinutes=secsRemaining/60;
  int runSeconds=secsRemaining%60;

  //Serial.print(" at time ");
  // char buf[21];
  // sprintf(buf,"%02d:%02d:%02d",runHours,runMinutes,runSeconds);
  // Serial.print(buf);
  // Serial.print(" (millis=");
  // Serial.print(t);
  // Serial.print("). ");

  //if (RAM.initFailure) {
  //  Serial.println(" Failed because of initial RAM failure");
  //} else {
    writeLogData(currentLogAddress, value, t);
  //  Serial.print(" Done.");
  //}
  //Serial.println("");

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

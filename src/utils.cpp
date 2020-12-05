#include "utils.h"

int UtilsClass::TC74_getTemperature() {
  // I2C based
  int temperature = -1;
    Wire.beginTransmission(72);
    //start the transmission

   byte val = 0;
    Wire.write(val);

    Wire.requestFrom(72, 1);
    if (Wire.available()) {
    temperature = Wire.read();
    //Serial.println(temperature);
    }
  Wire.endTransmission();
  return temperature;
  
}


UtilsClass UTILS;

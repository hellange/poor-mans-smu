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

int LM60_getTemperature(int analogPin) {
   analogReadRes(10);
  float maxNumber = 1023;//4095.0;
  float refV = 3.3; // LM60 connected to 3.3V
  float ar = analogRead(analogPin);
  float voltage = refV*(ar/maxNumber);
  // DM60 datasheet: Vout = (+6.25mV x t) +424mV 
  //                 => Vout - 424mV = +6.25mV x t 
  //                 => t = (Vout-424mV) / 6.25mV
  int temp = (voltage - 0.424) / 0.00625;
  //Serial.print(" ");
  //Serial.println(temp,3);
  return (int)temp;
}



UtilsClass UTILS;

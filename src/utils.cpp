#include "utils.h"
#include "Debug.h"

// To read temperature sensor on the heatsink.
// TODO: Make compatiple with more than one TC74
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
    //DEBUG.println(temperature);
    }
  Wire.endTransmission();
  return temperature;
  
}

// Read value from a LM60 that is mounted on the mainboard
// Pin nr on the current hardware prototyoe is 6
// Absolute temperature is not important, so no need to calibrate (yet)...
float UtilsClass::LM60_getTemperature(int analogPin) {

  // TODO: Maybe it does not make sense to use decimals
  //       as long as the LM60 only reports 0.5 degree resolution...

  
  if (millis() < sampleTimer + 5000) {
    // No need to go though the calulations too often
    return meanValueLM60;
  }
  analogReadRes(10);
  float maxNumber = 1023;//4095.0;
  float refV = 3.3; // LM60 connected to 3.3V on the prototype harware.
  float ar = analogRead(analogPin);
  float voltage = refV*(ar/maxNumber);
  // DM60 datasheet: Vout = (+6.25mV x t) +424mV 
  //                 => Vout - 424mV = +6.25mV x t 
  //                 => t = (Vout-424mV) / 6.25mV
  float temp = (voltage - 0.424) / 0.00625;
  //DEBUG.println(temp,3);
  if (temp>200 || temp < -10.0) {
      // value does not seem right... probaby not mounted.
      // TODO: Check if there are better ways to detect the sensor
      //DEBUG.print("System temperature monitor probably not detected!");
      return 99.9; // Just return a value that seems wrong...
  }
  meanValueLM60 = temp; // TODO: Calculate mean...
  return (float)temp;
}



UtilsClass UTILS;

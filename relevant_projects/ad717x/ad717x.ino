
int slaveSelectPin = 10;
#define ID 0x0CD9

unsigned int regValue;
#include <SPI.h>
#include "SMU_HAL_717x.h"

ADCClass ADC2;

void setup() {   
   Serial.begin(9600);

     
   pinMode(7,OUTPUT);

   pinMode(11,OUTPUT);
   pinMode(12,INPUT);
   pinMode(13,OUTPUT);
   disableOtherSPIunits();
   ADC2.init();
   delay(1000);
   Serial.println("Start measuring...");

}

void disableOtherSPIunits(){
   pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
}
    
void loop()
{

   long ret = ADC2.dataReady();
   if(ret < 0) {
   } else {
     float v = ADC2.measureMilliVoltage();

     Serial.print("raw ");
     Serial.print(AD7176_regs[4].value, HEX); 
     Serial.print("=");
     Serial.print(v);
     
     float offset = -0.0001;
     v=v+offset;
     Serial.print(" adjusted offset ");
     Serial.print(v);
  
     float gain_factor = 0.0484; // arduino 5V
     gain_factor = 0.0372; // teensy 3.3v
     v = v + v * gain_factor;
     Serial.print(" adjusted gain ");
     Serial.print(v);
     Serial.println(" mv");
     delay(1000);
   }
}

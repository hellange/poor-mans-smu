
int slaveSelectPin = 10;
#define ID 0x0CD9

unsigned int regValue;
#include <SPI.h>
#include "SMU_HAL_717x.h"

ADCClass ADC2;

void setup() {   
   Serial.begin(9600);

   pinMode(10,OUTPUT);
   pinMode(11,OUTPUT);
   pinMode(12,INPUT);
   pinMode(13,OUTPUT);
 
   ADC2.init();
   delay(1000);
   Serial.println("Start measuring...");

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
  
     float gain_factor = 0.0484;
     v = v + v * gain_factor;
     Serial.print(" adjusted gain ");
     Serial.print(v);
     Serial.println(" mv");
     delay(10);
   }
}

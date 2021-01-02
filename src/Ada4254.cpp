#include "Ada4254.h"
#include <Arduino.h>
#include <SPI.h>

Ada4254Class ADA4254;

void Ada4254Class::ada4254(bool on) {
 SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 
   uint8_t data =  SPI.transfer(0x2f | 0x80);
  uint8_t data2 =  SPI.transfer(0);

  // Serial.println(data);
      Serial.println(data2);

 data =  SPI.transfer(0x08);
 data2 =  SPI.transfer(0xff);

 data =  SPI.transfer(0x05);
 if (on) {
 data2 =  SPI.transfer(3);

 } else {
    data2 =  SPI.transfer(12);
 }


  delayMicroseconds(10); 
  digitalWrite(7, HIGH);
  SPI.endTransaction();
}


void Ada4254Class::ada4254_2() {
 SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10);
 
 SPI.transfer(0x08); // gpio output
 SPI.transfer(0xff);



  delayMicroseconds(10); 
  digitalWrite(7, HIGH);
  SPI.endTransaction();
}

void Ada4254Class::ada4254_3(bool on) {
 SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 



   SPI.transfer(0x05);
    if (on) {
   SPI.transfer(3);

 } else {
    SPI.transfer(12);
 }


  delayMicroseconds(10); 
  digitalWrite(7, HIGH);
  SPI.endTransaction();
}



void Ada4254Class::ada4254_5_gainx1d25() {
 SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10);

SPI.transfer(0x0E);  
//SPI.transfer(0b10000000);// *1.25
SPI.transfer(0b00000000);// *1


  delayMicroseconds(10); 
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

  SPI.endTransaction();
    delayMicroseconds(10);

}


void Ada4254Class::ada4254_5_gain() {
 SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10);


// 0 <4bit gain> x 00
SPI.transfer(0x00);
//SPI.transfer(0b00001000); 

//SPI.transfer(0b00010000); //gain 0.25 

//SPI.transfer(0b00011000); //gain 0.5  (0100)

//SPI.transfer(0b00100000); //gain 1  (0100)
//SPI.transfer(0b00101000); //gain 2  (0101)
//SPI.transfer(0b00110000); // gain 4
SPI.transfer(0b00111000); // gain 8
//SPI.transfer(0b01000000); // gain 16
//SPI.transfer(0b01001000); // gain 32
//SPI.transfer(0b01010000); // gain 64
//SPI.transfer(0b01011000); // gain 128 (1011)




  delayMicroseconds(10); 
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

  SPI.endTransaction();
    delayMicroseconds(10);

}


void Ada4254Class::ada4254_4() {
 SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 


   SPI.transfer(0x06);
   SPI.transfer(0b01100000); //input 1


  delayMicroseconds(10); 
  digitalWrite(7, HIGH);
  SPI.endTransaction();
}

// does not belong here !!!!
void Ada4254Class::relay(bool on) { 
  SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(4, LOW);

  if (on) {
    SPI.transfer(255);
  } else {
    SPI.transfer(0);
  }

  digitalWrite(4, HIGH);
  SPI.endTransaction();



}
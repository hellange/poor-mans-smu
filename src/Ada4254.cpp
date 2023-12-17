#include "Ada4254.h"
#include <Arduino.h>
#include <SPI.h>
#include <Debug.h>


Ada4254Class ADA4254;

//
//   Experimental testing of ADA4254 
//   TODO: Needs a proper cleanup !!!!!!
//


void Ada4254Class::printId() {
  SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(7, LOW);

  delayMicroseconds(10);
  //uint8_t data =  SPI.transfer(0x2f | 0x80);
  uint8_t data2 =  SPI.transfer(0);

  DEBUG.println("ADA4254 id:");
  DEBUG.println(data2, HEX);
  delayMicroseconds(10); 
  digitalWrite(7, HIGH);
  SPI.endTransaction();
}




void Ada4254Class::ada4254_check() {
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));


    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);
  delayMicroseconds(10);   //    ANALOG_ERR
  // disable all digital errors
  SPI.transfer(0x0B); // DIG ERR FILTER
  SPI.transfer(255);
  delayMicroseconds(10); 
  digitalWrite(7, HIGH);



    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);
  delayMicroseconds(10);  
  // clear all digital errors
  SPI.transfer(0x03); 
  SPI.transfer(0x08);
  delayMicroseconds(10); 
  digitalWrite(7, HIGH);



    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);
  delayMicroseconds(10);   //    ANALOG_ERR
  SPI.transfer(0x04 | 0x80);
  //uint8_t data2 =  SPI.transfer(0);
  //DEBUG.println("ADA4254 analog err:");
  //DEBUG.println(data2, BIN);
  delayMicroseconds(10); 
  digitalWrite(7, HIGH);


   digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);
  delayMicroseconds(10);   //    DIGITAL_ERR
  SPI.transfer(0x03 | 0x80);
  //uint8_t data2 =  SPI.transfer(0);
  //DEBUG.println("ADA4254 digital err:");
  //DEBUG.println(data2, BIN);
  delayMicroseconds(10); 
  digitalWrite(7, HIGH);


  SPI.endTransaction();






}






void Ada4254Class::ada4254_2(bool on) {
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10);
 
 SPI.transfer(0x08); // gpio output
 if (on) {
    SPI.transfer(0xff);

 }else {
    SPI.transfer(0xfb);

 }



  delayMicroseconds(10); 
  digitalWrite(7, HIGH);
  SPI.endTransaction();
}

void Ada4254Class::ada4254_3(bool on) {
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
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



void Ada4254Class::reset() {
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 

   SPI.transfer(0x01);
   SPI.transfer(0x01);

  delayMicroseconds(10); 
  digitalWrite(7, HIGH);


  SPI.endTransaction();
}



void Ada4254Class::ada4254_clear_analog_error() {
 
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 
   //SPI.transfer(0x03); // digital error register
   //SPI.transfer(0x08); // digital error register


   SPI.transfer(0x04); // analog error register
   SPI.transfer(0x80);

  delayMicroseconds(10); 
  digitalWrite(7, HIGH);






// MASK DIGITAL
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 
   //SPI.transfer(0x03); // digital error register
   //SPI.transfer(0x08); // digital error register
   SPI.transfer(0x0b); // digital mask
   SPI.transfer(0xff);
  delayMicroseconds(10); 
  digitalWrite(7, HIGH);


/*
// CLEAR DIGITAL
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(10); 
   //SPI.transfer(0x03); // digital error register
   //SPI.transfer(0x08); // digital error register


   SPI.transfer(0x03); // digital error register
   SPI.transfer(0x08);

  delayMicroseconds(10); 
  digitalWrite(7, HIGH);







*/

  SPI.endTransaction();

}






void Ada4254Class::ada4254_5_gainx1d25() {
 SPI.beginTransaction (SPISettings (500000, MSBFIRST, SPI_MODE0));
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

void Ada4254Class::ada4254_setgain(int gain) {
  SPI.beginTransaction (SPISettings (200000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

// 0 <4bit gain> x 00

SPI.transfer(0x00);

//SPI.transfer(0b00001000); 

//SPI.transfer(0b00010000); //gain 0.25 

//SPI.transfer(0b00011000); //gain 0.5  (0100)
if (gain == 1) {
  SPI.transfer(0b00100000); //gain 1  (0100)
}
else if (gain ==2) {
  SPI.transfer(0b00101000); //gain 2  (0101)
}
else if (gain ==4) {
  SPI.transfer(0b00110000); // gain 4
}
//SPI.transfer(0b00101000); //gain 2  (0101)
//SPI.transfer(0b00110000); // gain 4
//SPI.transfer(0b00111000); // gain 8
//SPI.transfer(0b01000000); // gain 16
//SPI.transfer(0b01001000); // gain 32
//SPI.transfer(0b01010000); // gain 64
//SPI.transfer(0b01011000); // gain 128 (1011)




  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

    delayMicroseconds(20);

  SPI.endTransaction();

}

void Ada4254Class::ada4254_5_gain() {
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

  delayMicroseconds(20);


// GPIO_DATA
SPI.transfer(0x05);  
SPI.transfer(0xff); // all gpio as output


 digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

    delayMicroseconds(20);

      digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

    delayMicroseconds(20);


// SET ERROR OUTPUT TO GPIO3

SPI.transfer(0x0C);  
SPI.transfer(0x08); 

 digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

    delayMicroseconds(20);

      digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

    delayMicroseconds(20);




SPI.transfer(0x0E);  
//SPI.transfer(0b10000000);// *1.25
SPI.transfer(0b00000000);// *1


 digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

    delayMicroseconds(20);

      digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(7, LOW);

    delayMicroseconds(20);

// 0 <4bit gain> x 00

SPI.transfer(0x00);

//SPI.transfer(0b00001000); 

//SPI.transfer(0b00010000); //gain 0.25 

//SPI.transfer(0b00011000); //gain 0.5  (0100)
//SPI.transfer(0b00100000); //gain 1  (0100)
//SPI.transfer(0b00101000); //gain 2  (0101)
//SPI.transfer(0b00110000); // gain 4
//SPI.transfer(0b00111000); // gain 8
//SPI.transfer(0b01000000); // gain 16
SPI.transfer(0b01001000); // gain 32
//SPI.transfer(0b01010000); // gain 64
//SPI.transfer(0b01011000); // gain 128 (1011)




  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(7, HIGH);

    delayMicroseconds(20);

  SPI.endTransaction();

}


void Ada4254Class::ada4254_4() {
 SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
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
  SPI.beginTransaction (SPISettings (2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(4, LOW);

  if (on) {
    SPI.transfer(255);
  } else {
    SPI.transfer(0);
  }

  digitalWrite(4, HIGH);
  SPI.endTransaction();



}



uint32_t relayTimer = millis();
bool relayState = false;
void Ada4254Class::indicateADA4254status() {
  //TODO: Clean up this ADA4254 prototyp/test mess !!!!
  //ADA4254.ada4254_5_gain();
  if (relayTimer+1000 < millis()) {
    relayState = !relayState;
    relayTimer = millis();
    // ADA4254.ada4254_id();
    ADA4254.ada4254_2(relayState);
    //ADA4254.ada4254_clear_analog_error();
    ADA4254.ada4254_check();
    // delay(500);
    //DEBUG.println("here333");
    //ADA4254.ada4254(relayState);
    //ADA4254.ada4254_5_gain();
  }
}


#include "Dac.h"

void DacClass::init() {
  pinMode(ADS1220_CS_PIN, OUTPUT);
  pinMode(ADS1220_DRDY_PIN, INPUT);
  ADS1220.begin();
  ADS1220.set_data_rate(DR_20SPS);
  ADS1220.set_pga_gain(PGA_GAIN_1);
  ADS1220.PGA_OFF();
}

bool DacClass::checkDataAvilable() {
  // TODO: Implement the Data Ready signal in hardware. Currently only using the SPI and poll
  //if((digitalRead(ADS1220_DRDY_PIN)) == LOW) //        Wait for DRDY to transition low
  { 
    SPI_RX_Buff_Ptr = ADS1220.Read_Data();
  }

  if(ADS1220.NewDataAvailable = true) {
    ADS1220.NewDataAvailable = false;
    MSB = SPI_RX_Buff_Ptr[0];    
    data = SPI_RX_Buff_Ptr[1];
    LSB = SPI_RX_Buff_Ptr[2];
    return true;
  } else {
    return false;
  }
}
float DacClass::smoothing(float average, int size, float value) {
  if (nrBeforeSmoothing > size){
       return (size * average + value ) / (size + 1);
  } else {
    nrBeforeSmoothing ++;
    return value;
  }
}

float DacClass::convertToMv() {

   long int bit32;
   long int bit24;
   byte *config_reg;
  

  
    bit24 = MSB;
    bit24 = (bit24 << 8) | data;
    bit24 = (bit24 << 8) | LSB;                                 // Converting 3 bytes to a 24 bit int
      
    /*if (MSB & 0x80)
      bit32 = (bit24 | 0xFF000000);             // Converting 24 bit two's complement to 32 bit two's complement
    else    
      bit32 = bit24;
    */
    
    bit24= ( bit24 << 8 );
    bit32 = ( bit24 >> 8 );                      // Converting 24 bit two's complement to 32 bit two's complement


    /* 
    config_reg = ADS1220.get_config_reg();
    Serial.print("Config Reg : ");
    Serial.print(config_reg[0],HEX);
    Serial.print(" ");  
    Serial.print(config_reg[1],HEX);
    Serial.print(" ");
    Serial.print(config_reg[2],HEX);
    Serial.print(" ");
    Serial.println(config_reg[3],HEX);
    */
    
//    Serial.print(MSB,HEX);
//    Serial.print(data,HEX);
//    Serial.print(LSB,HEX);
//    
//    Serial.print(vout);
//    Serial.print("    ");
//    Serial.print(bit32,HEX);
    
    return (float)((bit32*VFSR*1000)/FSR);     //In  mV
}

DacClass DAC;

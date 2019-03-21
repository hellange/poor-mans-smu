
#include "SMU_HAL_717x.h"
st_reg init_state[] = 
{
    {0x00, 1, 0, 0x00l,   "Stat_Reg "}, //Status_Register
    {0x01, 2, 0, 0x8000l, "ADCModReg"}, //ADC_Mode_Register
    {0x02, 2, 0, 0x1840l, "IfModeReg"}, //Interface_Mode_Register
    {0x03, 3, 0, 0x0000l, "Reg_Check"}, //Register_Check
    {0x04, 3, 0, 0x0000l, "ADC_Data "}, //Data_Register
    {0x06, 2, 0, 0x080Cl, "GPIO_Conf"}, //IOCon_Register
//     {0x06, 2, 0, 0x0800l, "GPIO_Conf"}, //IOCon_Register

    {0x07, 2, 0, 0x0000l, "ID_ST_Reg"}, //ID_st_reg
    {0x10, 2, 0, 0x8001l, "Ch_Map_0 "}, //CH_Map_1
    {0x11, 2, 0, 0x0000l, "Ch_Map_1 "}, //CH_Map_2
    {0x12, 2, 0, 0x0000l, "Ch_Map_2 "}, //CH_Map_3
    {0x13, 2, 0, 0x0000l, "Ch_Map_3 "}, //CH_Map_4
    {0x20, 2, 0, 0x1020l, "SetupCfg0"}, //Setup_Config_1   //ext ref
//    {0x20, 2, 0, 0x0020l, "SetupCfg0"}, //Setup_Config_1  unipolar

    {0x21, 2, 0, 0x1020l, "SetupCfg1"}, //Setup_Config_2
    {0x22, 2, 0, 0x1020l, "SetupCfg2"}, //Setup_Config_3
    {0x23, 2, 0, 0x1020l, "SetupCfg3"}, //Setup_Config_4
    
    //{0x28, 2, 0, 0x020Al, "FilterCf0"}, //Filter_Config_1
    {0x28, 2, 0, 0x0214l, "FilterCf0"}, //Filter_Config_1  // 5 pr sek
    
    
    {0x29, 2, 0, 0x0200l, "FilterCf1"}, //Filter_Config_2
    {0x2a, 2, 0, 0x0200l, "FilterCf2"}, //Filter_Config_3
    {0x2b, 2, 0, 0x0200l, "FilterCf3"}, //Filter_Config_4
    {0x30, 3, 0, 0l,      "Offset_0 "}, //Offset_1
    {0x31, 3, 0, 0l,      "Offset_1 "},  //Offset_2
    {0x32, 3, 0, 0l,      "Offset_2 "}, //Offset_3
    {0x33, 3, 0, 0l,      "Offset_3 "}, //Offset_4
    {0x38, 3, 0, 0l,      "Gain_0   "}, //Gain_1
    {0x39, 3, 0, 0l,      "Gain_1   "}, //Gain_2
    {0x3a, 3, 0, 0l,      "Gain_2   "}, //Gain_3
    {0x3b, 3, 0, 0l,      "Gain_3   "}, //Gain_4
    {0xFF, 1, 0, 0l,      "Comm_Reg "} //Communications_Register
};

float ADCClass::measureMilliVoltage() {
  AD7176_ReadRegister(&AD7176_regs[4]);
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;
  return v;
}

bool ADCClass::dataReady() {
  return AD7176_ReadRegister(&AD7176_regs[Status_Register]);
}

void ADCClass::init(){
  Serial.print("SETUP: ");
  
  Serial.println(AD7176_Setup());
  Serial.println("REGS:");

  // copy of AD7176 registers
  enum AD7176_registers regNr;
 
  for(regNr = 0; regNr < Offset_1; ++regNr) {
    //if (regNr == ADC_Mode_Register) continue;
    //if (regNr == Interface_Mode_Register) continue;
    //if (regNr == Register_Check) continue;
    //if (regNr == Data_Register) continue;
    Serial.print("Write ");
    Serial.print(init_state[regNr].name);
    Serial.print(" ");
    Serial.print(regNr);
    Serial.print(" ");
    Serial.print(init_state[regNr].value, HEX);
    Serial.print(" ");
    Serial.print(AD7176_WriteRegister(init_state[regNr]));
    Serial.print(" bytes. Read ");
    Serial.print(init_state[regNr].name);
    Serial.print(" ");

    Serial.print(AD7176_ReadRegister(&AD7176_regs[regNr]));
    Serial.print(" bytes: ");
    Serial.println(AD7176_regs[regNr].value, HEX);
  }
  AD7176_UpdateSettings();
  
}



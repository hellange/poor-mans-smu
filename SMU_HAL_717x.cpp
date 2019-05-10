
#include "SMU_HAL_717x.h"
static st_reg init_state[] = 
{
    {0x00, 1, 0, 0x00l,   "Stat_Reg "}, //Status_Register
    {0x01, 2, 0, 0x8000l, "ADCModReg"}, //ADC_Mode_Register 100uS delay
    {0x02, 2, 0, 0x0000l, "IfModeReg"}, //Interface_Mode_Register

    {0x03, 3, 0, 0x0000l, "Reg_Check"}, //Register_Check
    {0x04, 3, 0, 0x0000l, "ADC_Data "}, //Data_Register
    {0x06, 2, 0, 0x080Cl, "GPIO_Conf"}, //IOCon_Register
//     {0x06, 2, 0, 0x0800l, "GPIO_Conf"}, //IOCon_Register

    {0x07, 2, 0, 0x0000l, "ID_ST_Reg"}, //ID_st_reg
    
    {0x10, 2, 0, 0x8001l, "Ch_Map_0 "}, //CH_Map_1   ain1, ain0
    
    {0x11, 2, 0, 0x8043l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2
    //{0x11, 2, 0, 0x0001l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2

    {0x12, 2, 0, 0x0000l, "Ch_Map_2 "}, //CH_Map_3
    {0x13, 2, 0, 0x0000l, "Ch_Map_3 "}, //CH_Map_4
  //  {0x20, 2, 0, 0x0f00l, "SetupCfg0"}, //Setup_Config_1   //ext ref
   {0x20, 2, 0, 0x1f20l, "SetupCfg0"}, //Setup_Config_1  unipolar enable buffer

    {0x21, 2, 0, 0x1020l, "SetupCfg1"}, //Setup_Config_2
    {0x22, 2, 0, 0x1020l, "SetupCfg2"}, //Setup_Config_3
    {0x23, 2, 0, 0x1020l, "SetupCfg3"}, //Setup_Config_4
    
    //{0x28, 2, 0, 0x020Al, "FilterCf0"}, //Filter_Config_1
     //{0x28, 2, 0, 0x0214l, "FilterCf0"}, //Filter_Config_1  // 5 pr sek
     //  {0x28, 2, 0, 0x0213l, "FilterCf0"}, //Filter_Config_1  // 10 pr sek
    // {0x28, 2, 0, 0x0212l, "FilterCf0"}, //Filter_Config_1  // 16.66 pr sek
    // {0x28, 2, 0, 0x0211l, "FilterCf0"}, //Filter_Config_1  // 20 pr sek
     {0x28, 2, 0, 0x0210l, "FilterCf0"}, //Filter_Config_1  // 49.96 pr sek

    
    {0x29, 2, 0, 0x0214l, "FilterCf1"}, //Filter_Config_2
    {0x2a, 2, 0, 0x0214l, "FilterCf2"}, //Filter_Config_3
    {0x2b, 2, 0, 0x0214l, "FilterCf3"}, //Filter_Config_4
    {0x30, 3, 0, 0x800000l,      "Offset_0 "}, //Offset_1
    {0x31, 3, 0, 0x800000l,      "Offset_1 "},  //Offset_2
    {0x32, 3, 0, 0x800000l,      "Offset_2 "}, //Offset_3
    {0x33, 3, 0, 0x800000l,      "Offset_3 "}, //Offset_4
    {0x38, 3, 0, 0x555555l,      "Gain_0   "}, //Gain_1
    {0x39, 3, 0, 0x555555l,      "Gain_1   "}, //Gain_2
    {0x3a, 3, 0, 0x555555l,      "Gain_2   "}, //Gain_3
    {0x3b, 3, 0, 0x555555l,      "Gain_3   "}, //Gain_4
    {0xFF, 1, 0, 0l,      "Comm_Reg "} //Communications_Register
};

float ADCClass::measureMilliVoltage() {
  AD7176_ReadRegister(&AD7176_regs[4]);
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;
  return v;
}

float ADCClass::measureMilliVoltage2() {
  AD7176_ReadRegister(&AD7176_regs[4]);
  //Serial.print("BIN:");
  //Serial.println(AD7176_regs[4].value, BIN);
  size_t adc_value = AD7176_regs[4].value;
  //Serial.println(adc_value, HEX);
  
  // convert to 2s complement
  adc_value=adc_value^0x800000;
  bool neg = adc_value & 0x800000;
  
  //Serial.print("NEG");
  //Serial.println(neg);
  //Serial.println(adc_value, HEX);

    if (neg==1) {
        adc_value++;
        adc_value *= -1;
        adc_value = adc_value & 0x00ffffff;
    }
    
    //Serial.println(adc_value, HEX);
    //Serial.println((adc_value*VFSR*1000.0)/(float)FSR);
    
    //adc_value=0x15dddd;
    //float v = (adc_value*VFSR*1000.0);//   divided by FSR; 
      float v = (adc_value*VFSR*1000.0);//   divided by FSR;
      //(((long int)1<<23)-1) 
  v=v / (float) (((long int)1<<23)-1);
  //float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  //v=v-VREF*1000.0;
  if (neg==1) {
    v=v*-1.0;
  }
  return v;
}

int ADCClass::dataReady() {
   //return AD7176_ReadRegister(&AD7176_regs[Status_Register]);
     return AD7176_dataReady();

}

int ADCClass::init(){
  Serial.print("SETUP: ");
  
  Serial.println(AD7176_Setup());
  Serial.println("REGS:");

  // copy of AD7176 registers
  enum AD7176_registers regNr;
 
  for(int regNr = 0; regNr < Gain_3; ++regNr) {

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
    delay(10);
  }
  AD7176_UpdateSettings();
  return 0;
}



int8_t ADCClass::fltSetCommitVoltageSource(float fVoltage) {
   setValueV = fVoltage;
   nowValueV = fVoltage;
   return nowValueV;
 }
 
 int8_t ADCClass::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 
 float ADCClass::measureCurrent(){

//  float simulatedLoad = 10.0; //ohm
//  nowValueI = nowValueV / simulatedLoad;
//  
//  nowValueI =  nowValueI + (random(0, 100) / 1000000.0);
//  return nowValueI;

    AD7176_ReadRegister(&AD7176_regs[4]);
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;
  nowValueI = v/1000.0;
  return v;

  
 }

 boolean ADCClass::compliance(){
   return abs(setValueI) < abs(nowValueI);
 }

 float ADCClass::getSetValuemV(){
  return setValueV * 1000.0;
 }

 float ADCClass::getSetValuemA(){
  return setValueI * 1000.0;
 }

 

    

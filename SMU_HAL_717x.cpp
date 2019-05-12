
#include "SMU_HAL_717x.h"

float DACA_RANGE_LOW = -10;
float DACA_RANGE_HIGH = 10;





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
    // {0x28, 2, 0, 0x0214l, "FilterCf0"}, //Filter_Config_1  // 5 pr sek
    //   {0x28, 2, 0, 0x0213l, "FilterCf0"}, //Filter_Config_1  // 10 pr sek
    // {0x28, 2, 0, 0x0212l, "FilterCf0"}, //Filter_Config_1  // 16.66 pr sek
    {0x28, 2, 0, 0x0211l, "FilterCf0"}, //Filter_Config_1  // 20 pr sek
     //{0x28, 2, 0, 0x0210l, "FilterCf0"}, //Filter_Config_1  // 49.96 pr sek

    
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

  LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 0);  // initialising all channels to 0V - 5V range
  
  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x111); // initialize with a value to see that output works
  

  return 0;
}



/*
 *   Serial.println("|    0   |    0 - 5 V    |");
  Serial.println("|    1   |    0 - 10 V    |");
  Serial.println("|    2   |   -5 - +5 V   |");
  Serial.println("|    3   |  -10 - +10 V  |");
  Serial.println("|    4   | -2.5 - +2.5 V |");
  Serial.println("|    5   | -2.5 - +7.5 V |");

  Serial.print("\nEnter your choice: ");
  choice = read_int();
  Serial.println(choice);
  span = (uint32_t)(choice << 2);
 */

 // set
  float set_dac[]  = {0.0, 0.10000, 0.20000, 0.90000, 1000.00, 1200.00, 1600.00, 1800.00, 2000.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  
  // actual output
  float meas_dac[] = {0.0, 0.10000, 0.20000, 0.90000, 1000.03, 1200.00, 1599.89, 1799.86, 1999.85, 2999.59, 3999.39, 4999.12, 5998.93, 6998.66, 7998.46, 8193.95, 10000.00};

float nonlinear_comp(float milliVolt) {
   // Nonlinearity
   Serial.print("Looping up in comp table for ");
   Serial.print(milliVolt);
   Serial.println(" millivolt");
   float v = milliVolt;
  for (int i=0;i<16;i++) {
    if (v > meas_dac[i] && v <= meas_dac[i+1]) {
      float adj_factor_low = set_dac[i] - meas_dac[i];
      float adj_factor_high = set_dac[i+1] - meas_dac[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_dac[i+1] - set_dac[i];
      float partWithinRange = ( (v-set_dac[i]) / range); /* 0 to 1. Where then 0.5 is in the middle of the range */
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;
 
      Serial.print("meas:");  
      Serial.print(v, 3);
      Serial.print(", range:");  
      Serial.print(range, 3);
      Serial.print(", part:");  
      Serial.print(partWithinRange, 3);
      Serial.print(", factor:");  
      Serial.println(adj_factor, 3);

      Serial.flush();
      v = v + adj_factor; 
      
      return v;
    }
  } 
  Serial.println("no comp");
  return milliVolt;  
}

uint32_t voltage_to_code_adj(float dac_voltage, float min_output, float max_output, bool serialOut){
 dac_voltage = dac_voltage - 0.000250; // offset
  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0
 dac_voltage = dac_voltage * 0.99960;
Serial.print("voltage:");
Serial.print(dac_voltage);
Serial.println(" volt");

  dac_voltage = nonlinear_comp(dac_voltage * 1000.0) / 1000.0;

  return LTC2758_voltage_to_code(dac_voltage, min_output, max_output, serialOut);
}


int8_t ADCClass::fltSetCommitVoltageSource(float fVoltage) {
   setValueV = fVoltage;
   nowValueV = fVoltage;
   Serial.println(" XXXXXXXXXXXXXXXXXXXXXXXXX ");
    Serial.println(fVoltage);
    //SPAN
    //       Serial.println("|    0   |    0 - 5 V    |");
    //  Serial.println("|    1   |    0 - 10 V    |");
    //  Serial.println("|    2   |   -5 - +5 V   |");
    //  Serial.println("|    3   |  -10 - +10 V  |");
    //  Serial.println("|    4   | -2.5 - +2.5 V |");
    //  Serial.println("|    5   | -2.5 - +7.5 V |");
      uint32_t choice = 3;
      uint32_t span = (uint32_t)(choice << 2);
      
        LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);

      float v_adj = voltage_to_code_adj(fVoltage, DACA_RANGE_LOW, DACA_RANGE_HIGH, false);
       LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

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

 

    

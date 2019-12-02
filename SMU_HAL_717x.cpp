
#include "SMU_HAL_717x.h"
#include "Calibration.h"

static st_reg init_state[] = 
{
    {0x00, 1, 0, 0x00l,   "Stat_Reg "}, //Status_Register
    {0x01, 2, 0, 0x8000l, "ADCModReg"}, //ADC_Mode_Register
    //{0x01, 2, 0, 0x0040l, "ADCModReg"}, //ADC_Mode_Register - internal calibration

    {0x02, 2, 0, 0x0000l, "IfModeReg"}, //Interface_Mode_Register

    {0x03, 3, 0, 0x0000l, "Reg_Check"}, //Register_Check
    {0x04, 3, 0, 0x0000l, "ADC_Data "}, //Data_Register
    {0x06, 2, 0, 0x080Cl, "GPIO_Conf"}, //IOCon_Register
//     {0x06, 2, 0, 0x0800l, "GPIO_Conf"}, //IOCon_Register

    {0x07, 2, 0, 0x0000l, "ID_ST_Reg"}, //ID_st_reg

    // voltage meas
    {0x10, 2, 0, 0x8001l, "Ch_Map_0 "}, //CH_Map_1   ain1, ain0

    // current meas
    {0x11, 2, 0, 0x8043l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2
    //{0x11, 2, 0, 0x0001l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2

    {0x12, 2, 0, 0x0000l, "Ch_Map_2 "}, //CH_Map_3
    {0x13, 2, 0, 0x0000l, "Ch_Map_3 "}, //CH_Map_4

   {0x20, 2, 0, 0x1f00l, "SetupCfg0"}, //Setup_Config_1   //ext ref 
   //{0x20, 2, 0, 0x1f20l, "SetupCfg0"}, //Setup_Config_1  //int ref, unipolar 

    {0x21, 2, 0, 0x1020l, "SetupCfg1"}, //Setup_Config_2
    
    {0x22, 2, 0, 0x1020l, "SetupCfg2"}, //Setup_Config_3
    {0x23, 2, 0, 0x1020l, "SetupCfg3"}, //Setup_Config_4
    
    //{0x28, 2, 0, 0x020Al, "FilterCf0"}, //Filter_Config_1


       //  {0x28, 2, 0, 0x0216l, "FilterCf0"}, //Filter_Config_1  // 1.25 pr sek, for AD7172-2 only?
         //{0x28, 2, 0, 0x0215l, "FilterCf0"}, //Filter_Config_1  // 2.5 pr sek, for AD7172-2 only?
 //{0x28, 2, 0, 0x0214l, "FilterCf0"}, //Filter_Config_1  // 5 pr sek
   //  {0x28, 2, 0, 0x0213l, "FilterCf0"}, //Filter_Config_1  // 10 pr sek
    // {0x28, 2, 0, 0x0212l, "FilterCf0"}, //Filter_Config_1  // 16.66 pr sek
 {0x28, 2, 0, 0x0211l, "FilterCf0"}, //Filter_Config_1  // 20 pr sek
 //   {0x28, 2, 0, 0x0210l, "FilterCf0"}, //Filter_Config_1  // 49.96 pr sek
    // {0x28, 2, 0, 0x020fl, "FilterCf0"}, //Filter_Config_1  // 59.92 pr sek
    // {0x28, 2, 0, 0x020el, "FilterCf0"}, //Filter_Config_1  // 100 pr sek

    
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

bool full_board = true; // set to true to account for shunt and gain/offsets other places that dac/adc


float ADCClass::measureMilliVoltage() {
  AD7176_ReadRegister(&AD7176_regs[4]);
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;

  v = v / 0.8;  // funnel amplifier
  v = V_CALIBRATION.adjust(v);

  if (full_board == true) {
    v = v +3.0; // offset
    v = v*1.00034; // gain
  }
  return v;
}

int ADCClass::dataReady() {
  return AD7176_dataReady();
}

int ADCClass::init(){
  Serial.print("SETUP: ");
  AD7176_Reset;
  Serial.println(AD7176_Setup());
  Serial.println("REGS:");

  // copy of AD7176 registers
  enum AD7176_registers regNr;

 // Gain_3
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

  LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 3);  // initialising all channels to -10V - 10V range

  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x0); // init to 0;
  

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
 // float set_dac[]  = {-2000.00, -1000.00, 0.00, 900.00, 1000.00, 1100.00, 1200.00, 1600.00, 1800.00, 2000.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  
  // actual output
 // float meas_dac[] = {-1999.86, -0990.80, 0.00, 900.121, 1000.056, 1100.350, 1200.30, 1599.45, 1799.78, 1999.84, 2999.88, 3999.02, 4999.66, 5999.38, 6999.31, 7999.31, 9000.00, 10000.00};

/*
float nonlinear_comp(float milliVolt) {
   // Nonlinearity
   //todo: CHECK IF THIS WORKS 
   Serial.print("Looking up in comp table for ");
   Serial.print(milliVolt);
   Serial.println(" millivolt");
   float v = milliVolt;
  for (int i=0;i<18;i++) {
    if (v > meas_dac[i] && v <= meas_dac[i+1]) {
      float adj_factor_low = set_dac[i] - meas_dac[i];
      float adj_factor_high = set_dac[i+1] - meas_dac[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_dac[i+1] - set_dac[i];
      float partWithinRange = ( (v-set_dac[i]) / range); // 0 to 1. Where then 0.5 is in the middle of the range 
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;

      Serial.print("meas:");  
      Serial.print(v, 4);
      Serial.print(", range:");  
      Serial.print(range, 4);
      Serial.print(", part:");  
      Serial.print(partWithinRange, 4);
      Serial.print(", diff:");  
      Serial.print(adj_factor_diff, 4);
      Serial.print(", factor:");  
      Serial.println(adj_factor, 4);

      Serial.flush();
      v = v + adj_factor; 
      
      return v;
    }
  } 
  Serial.println("no comp");
  return milliVolt;  
}
*/
uint32_t voltage_to_code_adj(float dac_voltage, float min_output, float max_output, bool serialOut){

//if (CALIBRATION.useCalibratedValues == true) {
//  dac_voltage = nonlinear_comp(dac_voltage * 1000.0) / 1000.0;
//}
  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0
  dac_voltage = dac_voltage + 0.0; // offset
  dac_voltage = dac_voltage * 0.99978; // gain
  
  Serial.print("voltage:");
  Serial.print(dac_voltage);
  Serial.println(" volt");
  

dac_voltage = - dac_voltage; // analog part requires inverted input
  
  return LTC2758_voltage_to_code(dac_voltage, min_output, max_output, serialOut);
}


int8_t ADCClass::fltSetCommitVoltageSource(float v) {
  setValueV = v;
  //nowValueV = v;



  //SPAN 0 = 0 to +5V
  //     1 = 0 to +10V
  //     2 = -5 to +5 V
  //     3 = -10 to +10V
  //     4 = -2.5 to +2.5V
  //     5 = -2.5 to + 7.5V
  uint32_t choice = 3;
  float DAC_RANGE_LOW = -10.0;
  float DAC_RANGE_HIGH = 10.0;
  
  if (abs(v) <2.0) {   // can move to 2.5 if reference voltage is 5v
    choice = 4;
    DAC_RANGE_LOW = -2.5;
    DAC_RANGE_HIGH = 2.5;
  }
  
  uint32_t span = (uint32_t)(choice << 2);

  
  LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);

  
  float v_adj = voltage_to_code_adj(v, DAC_RANGE_LOW, DAC_RANGE_HIGH, true);
    


  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

 return setValueV;
 }
 
 int8_t ADCClass::fltSetCommitCurrentSource(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 
 float ADCClass::measureCurrent(int range){

    AD7176_ReadRegister(&AD7176_regs[4]);
    float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
    v=v-VREF*1000.0;
    // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
    if (full_board == true) {
      if (range == 1) {
        v=v/100.0; // 100 ohm shunt.
      }
      v=v/10.0; // x10 amplifier
  
      // account for gain in amps
      //v = v * 0.9941; // 1ohm shunt
      v = v * 0.9878; // 100ohm shunt
    }

    v = v / 0.8;  // funnel amplifier x0.8

    compliance = abs(setValueI) < abs(v/1000.0);
//    Serial.print("compliance setValueI:");  
//    Serial.print(abs(setValueI*1000.0));
//    Serial.print(", valueI:");
//    Serial.println(abs(v));

    return v;
 }

 float ADCClass::getSetValuemV(){
  return setValueV * 1000.0;
 }

 float ADCClass::getSetValuemA(){
  return setValueI * 1000.0;
 }

 

    

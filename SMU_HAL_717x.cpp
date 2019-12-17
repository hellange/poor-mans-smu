
#include "SMU_HAL_717x.h"
#include "Calibration.h"
#include "Stats.h"
#include "Filters.h"

static st_reg init_state[] = 
{
    {0x00, 1, 0, 0x00l,   "Stat_Reg "}, //Status_Register
    {0x01, 2, 0, 0x8000l, "ADCModReg"}, //ADC_Mode_Register
    //{0x01, 2, 0, 0x0040l, "ADCModReg"}, //ADC_Mode_Register - internal calibration

    {0x02, 2, 0, 0x0000l, "IfModeReg"}, //Interface_Mode_Register

    {0x03, 3, 0, 0x0000l, "Reg_Check"}, //Register_Check
    {0x04, 3, 0, 0x0000l, "ADC_Data "}, //Data_Register
    {0x06, 2, 0, 0x080Cl, "GPIO_Conf"}, //IOCon_Register   // both gpio output low
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
   // {0x28, 2, 0, 0x0210l, "FilterCf0"}, //Filter_Config_1  // 49.96 pr sek
     //{0x28, 2, 0, 0x020fl, "FilterCf0"}, //Filter_Config_1  // 59.92 pr sek
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

void ADCClass::setCurrentRange(int range) {
  current_range = range;
  if (range == 0) {
    AD7176_WriteRegister({0x06, 2, 0, 0x080Cl});
    digitalWrite(4, HIGH);
  } else {
    AD7176_WriteRegister({0x06, 2, 0, 0x080Dl});
    digitalWrite(4, LOW);
  }

}

float ADCClass::measureMilliVoltage() {
  AD7176_ReadRegister(&AD7176_regs[4]);

  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;

  v = v / 0.8;  // funnel amplifier
  v = V_CALIBRATION.adc_nonlinear_compensation(v);
  
  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board == true) {
    //v = v +3.0; // offset
    if (v>0) {
      v = v*1.00039; //gain on positive
      v = v*2.0; // divide by 2 in measurement circuit
    } else {
      v = v*1.00156; // gain on negative
      v = v*2.0; // divide by 2 in measurement circuit
    }
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


uint32_t sourcevoltage_to_code_adj(float dac_voltage, float min_output, float max_output, bool serialOut){

  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0
  dac_voltage = dac_voltage - 0.000825;
  if (dac_voltage > 0) {
    // positive
    dac_voltage = dac_voltage * 1.00103; // gain
  } else  {
    // negative
    dac_voltage = dac_voltage * 0.998750; // gain
  }
  
  //Serial.print("voltage:");
  //Serial.print(dac_voltage);
  //Serial.println(" volt");

  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board) {
    dac_voltage = dac_voltage + 0.002725; // additional offset (measured when connected to amplifier board)
    if (dac_voltage>0) {
      // positive
      dac_voltage = dac_voltage / 2.00265; // There is a apprx. /2 on the sense input in addition to gain
    } else {
      // negative
      dac_voltage = dac_voltage / 2.00048; // There is a apprx. /2 on the sense input in addition to gain 
    }
    dac_voltage = - dac_voltage; // analog part requires inverted input
  }

  return LTC2758_voltage_to_code(dac_voltage, min_output, max_output, serialOut);
}

uint32_t ADCClass::sourcecurrent_to_code_adj(float dac_voltage, float min_output, float max_output, bool serialOut){

  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0

//  if (dac_voltage > 0) {
//    // positive
//    dac_voltage = dac_voltage * 0.999785; // gain
//  } else  {
//    // negative
//    dac_voltage = dac_voltage * 0.998750; // gain
//  }
 

  if (full_board) {
    //dac_voltage = dac_voltage - 0.0004; // additional offset (measured when connected to amplifier board)
    if (current_range == 0) {
      dac_voltage = dac_voltage * 10.0; // with 1ohm shunt and x10 amplifier: 100mA is set by 1000mV
      dac_voltage = dac_voltage + 0.0008; // offset
      dac_voltage = dac_voltage * 1.137;
    } else if (current_range == 1) {
      dac_voltage = dac_voltage * 1000.0; // with 100ohm shunt and x10 amplifier: 1mA range is set by 1000mV
      dac_voltage = dac_voltage + 0.0010; // offset
      dac_voltage = dac_voltage * 0.975;
    }
   
    dac_voltage = - dac_voltage; // analog part requires inverted input
  }

  return LTC2758_voltage_to_code(dac_voltage, min_output, max_output, serialOut);
}

int8_t ADCClass::fltSetCommitVoltageSource(float milliVolt) {
  float v = milliVolt / 1000.0;
  setValueV = v; // use volt. TODO: change to millivolt ?

  float mv = milliVolt;
  if (V_CALIBRATION.useCalibratedValues == true) {
    mv = V_CALIBRATION.dac_nonlinear_compensation(mv);
  }

  v = mv / 1000.0;
  
  //SPAN 0 = 0 to +5V
  //     1 = 0 to +10V
  //     2 = -5 to +5 V
  //     3 = -10 to +10V
  //     4 = -2.5 to +2.5V
  //     5 = -2.5 to + 7.5V
  uint32_t choice = 3;
  float DAC_RANGE_LOW = -10.0;
  float DAC_RANGE_HIGH = 10.0;
  
  if (abs(v) <2.2) {   // can move to 2.5 if reference voltage is 5v
    choice = 4;
    DAC_RANGE_LOW = -2.5;
    DAC_RANGE_HIGH = 2.5;
  }
  
  uint32_t span = (uint32_t)(choice << 2);

  LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);

  float v_adj = sourcevoltage_to_code_adj(v, DAC_RANGE_LOW, DAC_RANGE_HIGH, false);
  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

 return setValueV;
 }

 int pulseTimer = millis();
 int pulseHigh = false;
 void ADCClass::pulse(float high, float low, int duration) {

   if (pulseHigh && pulseTimer+duration/2 < millis()) {
     pulseHigh = false;
     pulseTimer = millis();
     fltSetCommitVoltageSource(low);
     //Serial.println("Set pulse low");
   } else if (!pulseHigh && pulseTimer+duration/2 < millis()) {
     pulseHigh = true;
     pulseTimer = millis();
     fltSetCommitVoltageSource(high);
     //Serial.println("Set pulse high");

   }
 }

 float currentSweepValue = 0.0;
 int currentSweepDir = 1;
 void ADCClass::sweep(float high, float low, float step, int duration) {
   
   if (pulseTimer+duration/2 < millis()) {
     pulseTimer = millis();
     if (currentSweepDir == 1 && currentSweepValue > high) {
      currentSweepDir = -1;
     } else if (currentSweepDir == -1 && currentSweepValue < low) {
      currentSweepDir = +1;
     }
     currentSweepValue += step*currentSweepDir;

     fltSetCommitVoltageSource(currentSweepValue);
     
     //Serial.println("Set pulse low");
   } 
 }
 
 
 int8_t ADCClass::fltSetCommitCurrentSource(float milliVolt) {
  float v = milliVolt / 1000.0;
  setValueV = v; // use volt. TODO: change to millivolt ?

  float mv = milliVolt;
  v = mv / 1000.0;
  
  //SPAN 0 = 0 to +5V
  //     1 = 0 to +10V
  //     2 = -5 to +5 V
  //     3 = -10 to +10V
  //     4 = -2.5 to +2.5V
  //     5 = -2.5 to + 7.5V
  uint32_t choice = 3;
  float DAC_RANGE_LOW = -10.0;
  float DAC_RANGE_HIGH = 10.0;
  
//  if (abs(v) <2.2) {   // can move to 2.5 if reference voltage is 5v
//    choice = 4;
//    DAC_RANGE_LOW = -2.5;
//    DAC_RANGE_HIGH = 2.5;
//  }
  
  uint32_t span = (uint32_t)(choice << 2);

  LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);

  float v_adj = sourcecurrent_to_code_adj(v, DAC_RANGE_LOW, DAC_RANGE_HIGH, false);
  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  return setValueV;
 }
 
 
 int8_t ADCClass::fltSetCommitLimit(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 
 float ADCClass::measureCurrent(int range){

    AD7176_ReadRegister(&AD7176_regs[4]);
    float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
    v=v-VREF*1000.0;
    
    v = v / 0.8;  // funnel amplifier x0.8

    float i = v;
    // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
    if (full_board == true) {
      if (range == 1) {
        i=v/100.0; // 100 ohm shunt.
        i=i*1.05; 
      } else {
        i=i/1.04600; // 1ohm shunt + resistance in range switch mosfet
        i=i*0.883;
      }
      i=i/10.0; // x10 amplifier

      // account for resistor value not perfect
      if (range == 0) {
        i = i *   1.043; // 1ohm shunt, 1A range
        i = i -V_FILTERS.mean* 0.000075; // account for common mode voltage giving wrong current (give too high result)
      } else {
        i = i * 0.985; // 100ohm shunt, 10mA range
        i = i -V_FILTERS.mean* 0.000075; // account for common mode voltage giving wrong current (give too high result)      
      }
      
      
    }


    // TODO: replace with signal from actual circuit (current limit)
    compliance = abs(setValueI) < abs(i/1000.0);
//    Serial.print("compliance setValueI:");  
//    Serial.print(abs(setValueI*1000.0));
//    Serial.print(", valueI:");
//    Serial.println(abs(v));

    return i;
 }

 float ADCClass::getSetValuemV(){
  return setValueV * 1000.0;
 }

 float ADCClass::getLimitValue(){
  return setValueI * 1000.0;
 }

 

    

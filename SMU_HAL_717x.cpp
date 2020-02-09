
#include "SMU_HAL_717x.h"
#include "Calibration.h"
#include "Stats.h"
#include "Filters.h"

static st_reg init_state[] = 
{
    {0x00, 1, 0, 0x00l,   "Stat_Reg "}, //Status_Register

    //{0x01, 2, 0, 0xa000l, "ADCModReg"}, //ADC_Mode_Register, sincle cyc
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
    
    //              c means disable input buffers
    {0x20, 2, 0, 0x1c00l, "SetupCfg0"}, //Setup_Config_1   //ext ref 
   //{0x20, 2, 0, 0x1f20l, "SetupCfg0"}, //Setup_Config_1  //int ref, unipolar 

    {0x21, 2, 0, 0x1020l, "SetupCfg1"}, //Setup_Config_2
    
    {0x22, 2, 0, 0x1020l, "SetupCfg2"}, //Setup_Config_3
    {0x23, 2, 0, 0x1020l, "SetupCfg3"}, //Setup_Config_4
    
    //{0x28, 2, 0, 0x020Al, "FilterCf0"}, //Filter_Config_1


       //  {0x28, 2, 0, 0x0216l, "FilterCf0"}, //Filter_Config_1  // 1.25 pr sek, for AD7172-2 only?
         //{0x28, 2, 0, 0x0215l, "FilterCf0"}, //Filter_Config_1  // 2.5 pr sek, for AD7172-2 only?
 //{0x28, 2, 0, 0x0214l, "FilterCf0"}, //Filter_Config_1  // 5 pr sek
  //   {0x28, 2, 0, 0x0213l, "FilterCf0"}, //Filter_Config_1  // 10 pr sek
    // {0x28, 2, 0, 0x0212l, "FilterCf0"}, //Filter_Config_1  // 16.66 pr sek
 //{0x28, 2, 0, 0x0211l, "FilterCf0"}, //Filter_Config_1  // 20 pr sek
  // {0x28, 2, 0, 0x0210l, "FilterCf0"}, //Filter_Config_1  // 49.96 pr sek
     //{0x28, 2, 0, 0x020fl, "FilterCf0"}, //Filter_Config_1  // 59.92 pr sek
    {0x28, 2, 0, 0x020el, "FilterCf0"}, //Filter_Config_1  // 100 pr sek
//{0x28, 2, 0, 0x020dl, "FilterCf0"}, //Filter_Config_1   // 200 pr sek
//{0x28, 2, 0, 0x020cl, "FilterCf0"}, //Filter_Config_1   // 397.5 pr sek
//{0x28, 2, 0, 0x020bl, "FilterCf0"}, //Filter_Config_1  // 500 pr sek
//{0x28, 2, 0, 0x020al, "FilterCf0"}, //Filter_Config_1  // 1000 pr sek

    
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

void ADCClass::setSamplingRate(int value) {
  samplingRate = value;
  // Make sure the value is wrote into the DAC register. For example when reading current or voltage measurement.
  // I tried to set the register directly but ended up with problems (halt, stops, exception etc.) probably due to interrupt stuff...
  // TODO: Figure out a better way...
}


void ADCClass::writeSamplingRate() {
  if (oldSamplingRate == samplingRate) {
     return;
  }
  Serial.print("Setting new sampling rate:");
  Serial.println(samplingRate);
  oldSamplingRate = samplingRate;
  int value = samplingRate;
  // Note that two samples are needed for both voltage and current !
  // So the "visible" sample rate will be 1/2 of set. 


  // Experimenting with filters ????
  //15        SINC3_MAP_0
  //[14:12]   reserved
  //11        ENHFILTEN0
  //[10:8]    ENHFILT0
  //7         reserved
  //[6:5]     ORDER0      00 = Sinc5+Sinc1 (default), 11=sinc3
  //[4:0]     ORD0        sampling speed
  //
  //
  //unsigned int ENHFILTERN0 = 0x0800l;
  //unsigned int ENHFILT0_20SPS = 0x0500;

  
  if (value == 5) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0214l  });
  } else if (value == 10) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0213l  });
  } else if (value == 20) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0211l  });
  } else if (value == 50) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0210l  });
  } else if (value == 100) {
      AD7176_WriteRegister({0x28, 2, 0, 0x020el  });
  } else if (value == 200) {
      AD7176_WriteRegister({0x28, 2, 0, 0x020dl});
  } else if (value == 500) {
      AD7176_WriteRegister({0x28, 2, 0, 0x020bl}); 
  } else if (value == 1000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x020al}); 
  } else if (value == 2500) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0209l}); 
  } else if (value == 5000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0208l}); 
  } else if (value == 10000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0207l}); 
  } else if (value == 15625) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0206l}); 
  } else if (value == 25000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0205l}); 
  } else if (value == 31250) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0204l}); 
  } else if (value == 50000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0203l}); 
  } else {
    Serial.print("Illegal sample value:");
    Serial.println(value);
    Serial.flush();
  }
  
}
void ADCClass::setCurrentRange(CURRENT_RANGE range) {
  current_range = range;
  if (range == AMP1) {
   // Serial.print("range:");
   // Serial.println("AMP1");
   // AD7176_WriteRegister({0x06, 2, 0, 0x080Cl}); // GPIO pin
    digitalWrite(4, HIGH); 
  } else if (range == MILLIAMP10) {
   // AD7176_WriteRegister({0x06, 2, 0, 0x080Dl}); // GPIO pin
   digitalWrite(4, LOW);
    //Serial.print("range:");
    //Serial.println("MILLIAMP10");
  } else {
    Serial.println("ERROR: Unknown current range !!!");
      Serial.flush();

  }

}


float ADCClass::measureMilliVoltageRaw() {
  AD7176_ReadRegister(&AD7176_regs[4]);
     
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR);

  
  v=v-VREF*1000.0;
  writeSamplingRate();  // update sampling rate here seems to work. Doing randomly other places often fails.... for some reason...

  return v;
}

float ADCClass::measureMilliVoltage() {
 
  AD7176_ReadRegister(&AD7176_regs[4]);

  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;

  //v = v / 0.8;  // funnel amplifiersetNullValue x0.8
  v = v / 0.4;  // funnel amplifier x0.4
  
  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board == true) {
    //v = v +3.0; // offset
    if (v>0) {
      v = v*1.00039; //gain on positive
      v = v*2.0; // divide by 2 in measurement circuit
      v = v * V_CALIBRATION.getAdcGainCompPos();
    } else {
      v = v*1.00156; // gain on negative
      v = v*2.0; // divide by 2 in measurement circuit
      v = v * V_CALIBRATION.getAdcGainCompNeg();

    }
  }
  //Serial.print("Voltage nonlinear comp ");
  //Serial.print(v);
  v = V_CALIBRATION.adc_nonlinear_compensation(v);
  //Serial.print(" -> ");
  //Serial.println(v); 

  writeSamplingRate();  // update sampling rate here seems to work. Doing randomly other places often fails.... for some reason...
  return v;
}



int ADCClass::dataReady() {
  return AD7176_dataReady();
}

int ADCClass::init() {
  initADC();
  initDAC();
}

int ADCClass::initADC(){
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

  return 0;
}

int ADCClass::initDAC(){
  LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 3);  // initialising all channels to -10V - 10V range
  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x0); // init to 0;
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




int8_t ADCClass::fltSetCommitVoltageSource(float milliVolt, bool dynamicRange) {
  float v = milliVolt / 1000.0;
  setValueV = v; // use volt. TODO: change to millivolt ?

  float mv = milliVolt;
  if (V_CALIBRATION.useDacCalibratedValues == true) {
    mv = V_CALIBRATION.dac_nonlinear_compensation(mv);
  }
   
  float dac_voltage = mv / 1000.0;  // DAC code operates with V instead of mV 
  
  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0
  //dac_voltage = dac_voltage - 0.000825; // adjusting offset to null
  dac_voltage = dac_voltage + V_CALIBRATION.getDacZeroComp();
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
    dac_voltage = dac_voltage + 0.002725 + 0.000045; // additional offset (measured when connected to amplifier board)
    if (dac_voltage>0) {
      // positive
      dac_voltage = dac_voltage / 2.00245; // There is a apprx. /2 on the sense input in addition to gain
      dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompPos();
    } else {
      // negative
      dac_voltage = dac_voltage / 2.00048; // There is a apprx. /2 on the sense input in addition to gain 
      dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompNeg();

    }

    //SPAN 0 = 0 to +5V
    //     1 = 0 to +10V
    //     2 = -5 to +5 V
    //     3 = -10 to +10V
    //     4 = -2.5 to +2.5V
    //     5 = -2.5 to + 7.5V
    uint32_t choice = 3;
    float DAC_RANGE_LOW = -10.0;
    float DAC_RANGE_HIGH = 10.0;
    
    // NOTE: Causes spike when changing range during ramp/pulse !!!!
    //       Recreate by pulse with high and low in different ranges (ex 6.0V - 1.0V)
    // NOTE2: Changing range might require calibration for each range... How many of the available ranges should I use ?
    if (dynamicRange) {
      if (abs(dac_voltage) <=2.0) {   // using 4.096 ref. Can move to 2.4 if reference voltage is 5v.
        choice = 4;
        DAC_RANGE_LOW = -2.5;
        DAC_RANGE_HIGH = 2.5;
      } 
      else if (abs(dac_voltage) <=4.0) {   // using 4.096 ref. Can move to 4.9 if reference voltage is 5v.
        choice = 2;
        DAC_RANGE_LOW = -5.0;
        DAC_RANGE_HIGH = 5.0;
      }
    }
    uint32_t span = (uint32_t)(choice << 2);
    LTC2758_write(LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);
    
    dac_voltage = - dac_voltage; // amp board requires inverted input

    bool serialOut = false;
    float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);
    
    LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

    return setValueV;
  }
}

 
 
 int8_t ADCClass::fltSetCommitCurrentSource(float milliVolt) {
  float v = milliVolt / 1000.0;
  setValueV = v; // use volt. TODO: change to millivolt ?

  float mv = milliVolt;
  if (C_CALIBRATION.useDacCalibratedValues == true) {
    //Serial.println("USE CALIBRATED VALUES FOR CURRENT");
    //Serial.print(mv,6);
    mv = C_CALIBRATION.dac_nonlinear_compensation(mv);
    //Serial.print(" adjusted to ");
    //Serial.println(mv,6);

  }
  float dac_voltage = mv / 1000.0;  // DAC code operates with V instead of mV    

  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0

//  if (dac_voltage > 0) {
//    // positive
//    dac_voltage = dac_voltage * 0.999785; // gain
//  } else  {
//    // negative
//    dac_voltage = dac_voltage * 0.998750; // gain
//  }
 

  if (full_board) {
    if (current_range == AMP1) {
      dac_voltage = dac_voltage * 10.0; // with 1ohm shunt and x10 amplifier: 100mA is set by 1000mV
      dac_voltage = dac_voltage / 2.0; // if using 0.5ohm shunt instead of 1ohm shunt
      //dac_voltage = dac_voltage + 0.0008; // offset
      dac_voltage = dac_voltage + C_CALIBRATION.getDacZeroComp();

      dac_voltage = dac_voltage * 1.290;
      if (dac_voltage < 0) {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg();
      } else {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos();
      }

      //Serial.println("Calculating current to set for 1A range");
      //Serial.flush();

    } else if (current_range == MILLIAMP10) {
      dac_voltage = dac_voltage * 1000.0; // with 100ohm shunt and x10 amplifier: 1mA range is set by 1000mV
      dac_voltage = dac_voltage + 0.0010; // offset
      dac_voltage = dac_voltage * 0.975;
      if (dac_voltage < 0) {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg();
      } else {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos();
      }
      Serial.println("Calculating current to set for 100mA range");
      Serial.flush();

    } else {
      Serial.print("ERROR: Unknown current range ");
      Serial.println(current_range);
      Serial.flush();

    }
   
    dac_voltage = - dac_voltage; // analog part requires inverted input
  }

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

  bool serialOut = false;
  float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);

  LTC2758_write(LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  return setValueV;
 }
 
 
 int8_t ADCClass::fltSetCommitLimit(float fCurrent, int8_t up_down_both) {
  return setValueI = fCurrent;                         
 }

 
 float ADCClass::measureCurrent(CURRENT_RANGE range){

    AD7176_ReadRegister(&AD7176_regs[4]);
    float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
    v=v-VREF*1000.0;

    //v = v / 0.8;  // funnel amplifier x0.8
    v = v / 0.4;  // funnel amplifier x0.4

    float i = v;
    // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
    if (full_board == true) {
      if (range == MILLIAMP10) {
        i=v/100.0; // 100 ohm shunt.
        i=i*1.045; 
      } else {
        i=i/1.04600; // 1ohm shunt + resistance in range switch mosfet
        i=i*2.0; // if 0.5 ohm shunt instead of ohm shunt;
        i=i*0.776;
        if (i>0) {
          i = i * C_CALIBRATION.getAdcGainCompPos();
        } else {
          i = i * C_CALIBRATION.getAdcGainCompNeg();
        }
      }
      i=i/10.0; // x10 amplifier

      // account for resistor value not perfect
      if (range == AMP1) {
        i = i *   1.043; // 1ohm shunt, 1A range
        i = i -V_FILTERS.mean* 0.000042; // account for common mode voltage giving wrong current (give too high result)
      } else {
        i = i * 0.985; // 100ohm shunt, 10mA range
        i = i -V_FILTERS.mean* 0.0000440; // account for common mode voltage giving wrong current (give too high result)      
      }
      
      
    }

    //Serial.print("Current nonlinear comp ");
    //Serial.print(i);
    i = C_CALIBRATION.adc_nonlinear_compensation(i);
    //Serial.print(" -> ");
    //Serial.println(i);
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

 

    

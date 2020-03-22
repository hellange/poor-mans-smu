
#include "SMU_HAL_717x.h"
#include "Calibration.h"
#include "Stats.h"
#include "Filters.h"

  float R_shunt_1A = 0.50; //0.15;
  float R_mosfetSwitch = 0.025; // ?? TODO: What's its real resistance ? Does it change much ? Should we use relay for precision ?

        
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
    
    //              c means disable ref? input buffers
  {0x20, 2, 0, 0x1c00l, "SetupCfg0"}, //Setup_Config_1   //ext ref 
//{0x20, 2, 0, 0x1c20l, "SetupCfg0"}, //Setup_Config_1  //int ref, unipolar 

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


bool gpio0 = false;
bool gpio1 = false;
void ADCClass::setGPIO(int nr, bool on) {

  AD7176_ReadRegister(&AD7176_regs[6]); // GPIO register
  uint32_t v = AD7176_regs[6].value;
//  Serial.print("Set gpio");
//  Serial.print(nr);
//  Serial.print(" to ");
//  Serial.println(on==true?"true":"false");
//  Serial.print("GPIO register before:");
//  Serial.println(v, BIN);
  
  v = v&0xfff0;
 
  //TODO: Clean up this bit setting mess...
  if (nr==0 && on) {
    gpio0 = true;
  }
  if (nr == 0 && !on) {
    gpio0 = false;
  }
  if (nr==1 && on) {
    gpio1 = true;
  }
  if (nr == 1 && !on) {
    gpio1 = false;
  }
  byte b;
  if (gpio0 == false && gpio1 == false) {
    b = 0xCl;
  }
  if (gpio0 == true && gpio1 == false) {
    b = 0x0Dl;
  }
  if (gpio0 == false && gpio1 == true) {
    b = 0x0El;
  }
  if (gpio0 == true && gpio1 == true) {
    b = 0x0Fl;
  }
  v = v + b;

//  Serial.print("GPIO register after:");
//  Serial.println(v, BIN);
  
  AD7176_WriteRegister({0x06, 2, 0, v}); 
  
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
    v = v*2.0; // divide by 2 in measurement circuit
    if (v>0) {
      v = v * V_CALIBRATION.getAdcGainCompPos();
    } else {
      v = v * V_CALIBRATION.getAdcGainCompNeg();
    }
    v=v * 0.9970;
    v=v * 1.0255;
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
  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 3);  // initialising all channels to -10V - 10V range
  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x0); // init to 0;

    LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 3);  // initialising all channels to -10V - 10V range
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x6666); // init to some value;
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


bool ADCClass::use100uVSetResolution() {
  if (DAC_RANGE_LOW == -10.0) {
    return false;
  } else {
    return true;
  }
}



int8_t ADCClass::fltSetCommitVoltageSource(float milliVolt, bool dynamicRange) {
  float v = milliVolt / 1000.0;
  setValueV = v; // use volt. TODO: change to millivolt ?

  float mv = milliVolt;
  if (V_CALIBRATION.useDacCalibratedValues == true) {
    mv = V_CALIBRATION.dac_nonlinear_compensation(mv);
  }
   
  float dac_voltage = mv / 1000.0;  // DAC code operates with V instead of mV 
 
  dac_voltage = dac_voltage + V_CALIBRATION.getDacZeroComp();


  if (dac_voltage > 0) {
    // positive
    //dac_voltage = dac_voltage * 1.00103; // gain
  } else  {
    // negative
    //dac_voltage = dac_voltage * 0.998750; // gain
  }
  
  //Serial.print("voltage:");
  //Serial.print(dac_voltage);
  //Serial.println(" volt");

  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board) {
    
    dac_voltage = dac_voltage * 1.01;
    dac_voltage = dac_voltage * 0.975;


    if (dac_voltage>0) {
      // positive
      dac_voltage = dac_voltage / 2.00; // There is a apprx. /2 on the sense input in addition to gain
      dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompPos();
    } else {
      // negative
      dac_voltage = dac_voltage / 2.00; // There is a apprx. /2 on the sense input in addition to gain 
      dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompNeg();

    }

    //SPAN 0 = 0 to +5V
    //     1 = 0 to +10V
    //     2 = -5 to +5 V
    //     3 = -10 to +10V
    //     4 = -2.5 to +2.5V
    //     5 = -2.5 to + 7.5V
    uint32_t choice = 3;
    DAC_RANGE_LOW = -10.0;
    DAC_RANGE_HIGH = 10.0;
    
    // NOTE: Causes spike when changing range during ramp/pulse !!!!
    //       Recreate by pulse with high and low in different ranges (ex 6.0V - 1.0V)
    // NOTE2: Changing range might require calibration for each range... How many of the available ranges should I use ?
    if (dynamicRange) {
      if (abs(dac_voltage) <=2.3) {   // Can theoretically move to 2.5 if reference voltage is 5v, but let's have some margins...
        choice = 4;
        DAC_RANGE_LOW = -2.5;
        DAC_RANGE_HIGH = 2.5; 
      } 
      else if (abs(dac_voltage) <=4.6) {   // Can theoretically move to 5.0 if reference voltage is 5v.
        choice = 2;
        DAC_RANGE_LOW = -5.0;
        DAC_RANGE_HIGH = 5.0;
      }
    } else {
      
    }
    uint32_t span = (uint32_t)(choice << 2);
    LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);
    
    dac_voltage = - dac_voltage; // amp board requires inverted input

    bool serialOut = false;
    float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);
    
    LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

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

//  dac_voltage = dac_voltage * 5.0/4.096; // using 4.096 ref instead of 5.0

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

      
      dac_voltage = dac_voltage - 0.0029; // offset
      dac_voltage = dac_voltage + C_CALIBRATION.getDacZeroComp();

      dac_voltage = dac_voltage * 0.9; // TODO: DO above calculations with the mosfet resistance in account...
      if (dac_voltage < 0) { 
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg();
      } else {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos();
      }


      dac_voltage = dac_voltage * 3.125; // after using 3 opamp diff amplifier before 1997-3.... hmm...

 dac_voltage = dac_voltage *1.15;
 dac_voltage = dac_voltage *1.023;
      //Serial.println("Calculating current to set for 1A range");
      //Serial.flush();

    } else if (current_range == MILLIAMP10) {
      
      
      dac_voltage = dac_voltage * 1000.0; // with 100ohm shunt and x10 amplifier: 1mA range is set by 1000mV
      //TODO: Add daczerocomp also for 10mA range. Currently only imlemented for 1A range ?
      Serial.print("Setting:");
      Serial.print(dac_voltage);
      Serial.println("mV in DAC for 10mA range....");

      dac_voltage = dac_voltage * 0.970;

      dac_voltage = dac_voltage * 3.125; // after using 3 opamp diff amplifier before 1997-3.... hmm...
 dac_voltage = dac_voltage /1.015;

      if (dac_voltage < 0) {
        //dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg(); //TODO: Should differ between ranges !
      } else {
        //dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos(); //TODO: Should differ between ranges !
      }

      //dac_voltage = dac_voltage + C_CALIBRATION.getDacZeroComp() * 5000.0; //TODO: Should differ between ranges !

           // dac_voltage = dac_voltage -37.0; // zero comp !

      Serial.print("Adjusted to :");
      Serial.print(dac_voltage);
      Serial.println("mV in DAC for 10mA range....");
      
       
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

  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);

  bool serialOut = false;
  float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);

  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  return setValueV;
 }
 


  
 int8_t ADCClass::fltSetCommitVoltageLimit(float fCurrent, int8_t up_down_both) {

//  float DAC_RANGE_LOW = 0.0;
//  float DAC_RANGE_HIGH = 10.0;
//  uint32_t choice = 1;
//  uint32_t span = (uint32_t)(choice << 2);
   
  bool serialOut = false;
  
  float dac_voltage;
  
  dac_voltage = fCurrent;
  dac_voltage = dac_voltage * 1000.0; // converted to mV
  Serial.print("Asking for voltage limit ");
  Serial.print(fCurrent);
  Serial.print(" mV, converting to ");
  Serial.print(dac_voltage);
  Serial.println(" mV out from DAC");

  // TODO: Use more adjustments for inaccuracies ? Can we share the ones that are used in souring voltage ?
  dac_voltage = dac_voltage / 2.0; // divide by two in voltage measurement circuit
  dac_voltage = dac_voltage * 1.0025;
  dac_voltage = dac_voltage * 1.016;
  dac_voltage = dac_voltage * 1.00025;
  dac_voltage = dac_voltage * 1.0005;


  dac_voltage = dac_voltage -0.78 / 2.0; // TODO: Remove hardcoded offset for limit !  Add to calibration store as other comp values

  dac_voltage = dac_voltage / 1000.0;


 //SPAN 0 = 0 to +5V
    //     1 = 0 to +10V
    //     2 = -5 to +5 V
    //     3 = -10 to +10V
    //     4 = -2.5 to +2.5V
    //     5 = -2.5 to + 7.5V
    uint32_t choice = 1;
    float DAC_RANGE_LOW = 0.0;
    float DAC_RANGE_HIGH = 10.0;
    
   
    // NOTE2: Changing range might require calibration for each range... How many of the available ranges should I use ?
    bool dynamicRange = true;
    if (dynamicRange) {
      if (abs(dac_voltage) <=4.0) {   
        choice = 0;
        DAC_RANGE_LOW = 0.0;
        DAC_RANGE_HIGH = 5;
        Serial.println("USING LIMIT WITH DAC VOLTAGE BETWEEN 0 and 5");
      } 
     
    }
    uint32_t span = (uint32_t)(choice << 2);

    
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);
  float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  Serial.println("SETTING LIMIT");
  Serial.println(dac_voltage);
  return setValueI = fCurrent;   // TODO: Change this concept !!!!                       
 }
 
 int8_t ADCClass::fltSetCommitCurrentLimit(float fCurrent, int8_t up_down_both) {

  float DAC_RANGE_LOW = 0.0;
  float DAC_RANGE_HIGH = 10.0;
   uint32_t choice = 1;
   uint32_t span = (uint32_t)(choice << 2);
   
  bool serialOut = false;
  
  float dac_voltage;
  if (current_range == MILLIAMP10) {
    dac_voltage = fCurrent * 1000.0;
    Serial.print("Asking for 10mA range current limit ");
    Serial.print(fCurrent);
    Serial.print(" mA, converting to ");
    Serial.print(dac_voltage);
    Serial.println(" mV out from DAC (10mA range)");
  } else {
    
    dac_voltage = 10.0* fCurrent * (R_shunt_1A + R_mosfetSwitch);
    Serial.print("Asking for 1A range current limit ");
    Serial.print(fCurrent);
    Serial.print(" mA, converting to ");
    Serial.print(dac_voltage);
    Serial.println(" mV out from DAC (1A range)");
  }

  //dac_voltage = dac_voltage - 0.753; 
  //TODO: Account for more amplification in current after adding two opamps in front of 1997-3 .....  hmmmm  
  dac_voltage = dac_voltage *3.125;

  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);
  float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  Serial.println("SETTING LIMIT");
  Serial.println(dac_voltage);
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
        //i=i*0.79;// apprx.... why? 
        i=i/3.125; // After using two opamps in fromt of 1997-3....   Why did that give 3.125 gain ????

        i=i*1.05;

      } else {
        //i=i/1.04600; // 1ohm shunt + resistance in range switch mosfet
       // i=i*2.0; // if 0.5 ohm shunt instead of ohm shunt;

      
        i=i/(R_shunt_1A + R_mosfetSwitch); 
        
        //i=i*0.79; // apprx.... why ?
        if (i>0) {
          i = i * C_CALIBRATION.getAdcGainCompPos();
        } else {
          i = i * C_CALIBRATION.getAdcGainCompNeg();
        }

              i=i/3.125; // After using two opamps in fromt of 1997-3....   Why did that give 3.125 gain ????

      }
      i=i/10.0; // x10 amplifier


      // account for resistor value not perfect
      if (range == AMP1) {
       // i = i *   1.043; // 1ohm shunt, 1A range
        //i = i -V_FILTERS.mean* 0.000042; // account for common mode voltage giving wrong current (give too high result)
      } else {
       // i = i * 0.985; // 100ohm shunt, 10mA range
        //i = i -V_FILTERS.mean* 0.0000440; // account for common mode voltage giving wrong current (give too high result)      
      }
      
      
    }

    //Serial.print("Current nonlinear comp ");
    //Serial.print(i);
//    i = C_CALIBRATION.adc_nonlinear_compensation(i);
    //Serial.print(" -> ");
    //Serial.println(i);
    
    // dummy!
    // compiance = abs(setValueI) < abs(i/1000.0);
    // from HW signal !
    compliance = (digitalRead(2) == LOW or digitalRead(3) == LOW);

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

 

    


#include "SMU_HAL_717x.h"
#include "Calibration.h"
#include "Stats.h"
#include "Filters.h"
#include "Debug.h"

  float R_shunt_1A = 0.50; //0.15;
  float R_shunt_10mA = 100.0;
  float R_mosfetSwitch = 0.005; // ?? TODO: What's its real resistance ? Does it change much ? Should we use relay for precision ?

  // There is a apprx. /2 on the sense input. This means that the voltage from DAC must be half of the expected output
  //float voltageInputDividerCompensation = 0.5; 
  float voltageInputDividerCompensation = 1.0; // no divider
        
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

    {0x07, 1, 0, 0x0000l, "ID_ST_Reg"}, //ID_st_reg

    // voltage meas
    {0x10, 2, 0, 0x8001l, "Ch_Map_0 "}, //CH_Map_1   ain1, ain0
    //{0x10, 2, 0, 0x8000l, "Ch_Map_0 "}, //CH_Map_1  both to ain0, same as short circuit ?

    // current meas
    //{0x11, 2, 0, 0x0043l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2
    {0x11, 2, 0, 0x8043l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2

    //{0x11, 2, 0, 0x0001l, "Ch_Map_1 "}, //CH_Map_2   ain3, ain2
       // {0x11, 2, 0, 0x82c5l, "Ch_Map_1 "}, //CH_Map_2   ref- ref+
       // {0x11, 2, 0, 0x8232l, "Ch_Map_1 "}, //CH_Map_2   temp- temp+


    {0x12, 2, 0, 0x0000l, "Ch_Map_2 "}, //CH_Map_3
    {0x13, 2, 0, 0x0000l, "Ch_Map_3 "}, //CH_Map_4
    
    //              c means disable ref? input buffers
    
  //  {0x20, 2, 0, 0x1300l, "SetupCfg0"}, //Setup_Config_1   //ext ref, dis refbuffer

 //{0x20, 2, 0, 0x1c00l, "SetupCfg0"}, //Setup_Config_1   //ext ref, dis buffer

{0x20, 2, 0, 0x1f00l, "SetupCfg0"}, //Setup_Config_1   //ext ref, enable buffer
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

void ADCClass::setVrefMv(double vrefMv) {
  vref = vrefMv;
  VFSR = vref;
}
void ADCClass::disable_ADC_DAC_SPI_units(){
   pinMode(7,OUTPUT); // mux master chip select
   digitalWrite(7, HIGH); // comment out if running display on spi1
}

void ADCClass::setSamplingRate(int value) {
  samplingRate = value;
  // Make sure the value is wrote into the DAC register. For example when reading current or voltage measurement.
  // I tried to set the register directly but ended up with problems (halt, stops, exception etc.) probably due to interrupt stuff...
  // TODO: Figure out a better way...
}
int ADCClass::getSamplingRate() {
  return samplingRate;
}

void ADCClass::updateSettings() {
  if (enableCurrentMeasurement && enableVoltageMeasurement) {
      AD7176_WriteRegister({0x10, 2, 0, 0x8001l  }); // Channel 1 enable input 0,1 voltage
      AD7176_WriteRegister({0x11, 2, 0, 0x8043l  }); // Channel 2 enable input 2,3 current
  } else {
    if (enableCurrentMeasurement) {
      AD7176_WriteRegister({0x10, 2, 0, 0x8043l  });  // select input 2,3 (current) input for channel 1
      AD7176_WriteRegister({0x11, 2, 0, 0x0001l  });  // disable channel 2
    } 
    else if (enableVoltageMeasurement) {
      AD7176_WriteRegister({0x10, 2, 0, 0x8001l  }); // select input 0,1 (voltage) input for channel 1
      AD7176_WriteRegister({0x11, 2, 0, 0x0043l  }); // disable channel 2

    } 
  }

}


// -----  Internal short circuit of inputs---
void ADCClass::shortAdcInput(bool setShort) {
  // for voltage measurement
  shortSetting = setShort;
}

void ADCClass::writeShortSetting() {
if (shortSetting != oldShortSetting) {
    oldShortSetting = shortSetting;
    //update shortADC here as well. TODO: Move
    if (shortSetting) {
        AD7176_WriteRegister({0x10, 2, 0, 0x8000l  });
    } else {
        AD7176_WriteRegister({0x10, 2, 0, 0x8001l  });
    }
  }
}


// -----  internal ref or not ---
void ADCClass::internalRefInput(bool internalRef_) {
  internalRef = internalRef_;
}

void ADCClass::writeRefInputSetting() {
if (internalRef != oldInternalRef) {
    oldInternalRef = internalRef;
    if (internalRef) {
        AD7176_WriteRegister({0x20, 2, 0, 0x1c20l  });
        setVrefMv(2.5);
    } else {
        AD7176_WriteRegister({0x20, 2, 0, 0x1f00l  });
        setVrefMv(VREF_EXTERNAL_CAL);
    }
  }
}

void ADCClass::writeSamplingRate() {

  if (oldSamplingRate == samplingRate) {
     return;
  }
  DEBUG.print("Setting new sampling rate:");
  DEBUG.println(samplingRate);
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


  if (value == 1) { //1.25, AD7172 only
      AD7176_WriteRegister({0x28, 2, 0, 0x0216l  });
  }
  else if (value == 2) { //2.5, AD7172 only
      AD7176_WriteRegister({0x28, 2, 0, 0x0215l  });
  }
  
  else 
  
  
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
      AD7176_WriteRegister({0x28, 2, 0, 0x0204l}); // sinc3 ?
  } else if (value == 50000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0203l}); 
  } else {
    DEBUG.print("Illegal sample value:");
    DEBUG.println(value);
    DEBUG.flush();
  }
  
}



//TODO: GPIO is the how the io is implemented. Rename to what the function does
bool gpio0 = false;
bool gpio1 = false;
void ADCClass::setGPIO(int nr, bool on) {

  AD7176_ReadRegister(&AD7176_regs[6]); // GPIO register
  int32_t v = AD7176_regs[6].value;
//  DEBUG.print("Set gpio");
//  DEBUG.print(nr);
//  DEBUG.print(" to ");
//  DEBUG.println(on==true?"true":"false");
//  DEBUG.print("GPIO register before:");
//  DEBUG.println(v, BIN);
  
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

//  DEBUG.print("GPIO register after:");
//  DEBUG.println(v, BIN);
  
  AD7176_WriteRegister({0x06, 2, 0, v}); 
  
}

CURRENT_RANGE ADCClass::getCurrentRange() {
  return current_range;
}

void ADCClass::setCurrentRange(CURRENT_RANGE range, OPERATION_TYPE operationType) {
  current_range = range;

  // 1) When switching current range while in sourcing voltage mode, the voltage from current measurement circuit will change.
  // The voltage output from limit controlling DAC must be adjusted accordingly...    
  // 2) When switching current range while in current source mode, the voltage from source controlling DAC must be changed accordingly.
  //
  // This can cause glitches !!!! 
  // TODO: Needs investigation on how this can be done properly !


  if (range == AMP1) {    
    digitalWrite(4, HIGH); 
    setGPIO(1,1); // When GPIO controls the sense relay
    if (operationType == SOURCE_VOLTAGE) {
      DEBUG.println("SMU setCurrentRange to 1A (source voltage)");
      //DEBUG.println("Switching current range (to A) while in voltage source must also update DAC output for limiting circuit.");
      delay(1); // WARNING !!!! Had to add delay here to avoid voltage drop when changing from 10mA to 1A.
               //              Why?
               //              TODO: Find out why setting the current limit right after switch causes spike !!!!
               //              With the 10ms delay, there is 10ms where the current limit is too high (as set for the 10mA range!)

               // Test this by setting for example 1 volt. Then switch between ranges and verify that the output voltage 
               // does not change significantly. I tested with DMM6500 21000 readings pr sek. You can try an oscilloscope.
               //
               // I have verified that its the current limit that kicks in briefly. How to fix ?
               
      fltSetCommitCurrentLimit(setLimit_micro, _SOURCE_AND_SINK);//printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
    } 
    else {
            DEBUG.println("SMU setCurrentRange to 1A (source current)");

      //DEBUG.println("Switching current range (to A) while in current source must also update DAC output");
      fltSetCommitCurrentSource(setValue_micro);
    }


  } else if (range == MILLIAMP10) {
   if (operationType == SOURCE_VOLTAGE) {
     DEBUG.println("SMU setCurrentRange to 10mA (source voltage)");
     DEBUG.println("Switching current range (to 10mA) while in voltage source must also update DAC output for limiting circuit.");
     fltSetCommitCurrentLimit(setLimit_micro, _SOURCE_AND_SINK);// printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
   } 
   else {
     DEBUG.println("SMU setCurrentRange to 10mA (source current)");
     DEBUG.println("Switching current range (to 10mA) while in current source must also update DAC output");
     fltSetCommitCurrentSource(setValue_micro);
   }
   digitalWrite(4, LOW);
   setGPIO(1,0); // When GPIO controls the sense relay

  } else {
    DEBUG.println("ERROR: Unknown current range !!!");
    DEBUG.flush();
  }

}


double ADCClass::measureMilliVoltageRaw() {

  AD7176_ReadRegister(&AD7176_regs[4]);
     
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR);

  v = v - vref*1000.0;
  writeSamplingRate();  // update sampling rate here seems to work. Doing randomly other places often fails.... for some reason...

  return v;
}

bool ADCClass::hasCompliance() {
  return (digitalRead(2) == LOW or digitalRead(3) == LOW);
}

double ADCClass::measureMilliVoltage() {

  // AD7176_ReadRegister(&AD7176_regs[6]);
  // int r =  AD7176_regs[6].value;
  // Serial.print("ID:");  
  // Serial.println(r, HEX);     

  AD7176_ReadRegister(&AD7176_regs[4]);

  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-vref*1000.0;

  //v = v / 0.8;  // funnel amplifiersetNullValue x0.8
  v = v / 0.4;  // funnel amplifier x0.4
    
  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board == true) {
    
    if (voltageMeasurementGainX2) {
      v = v / 2.0; // divide by 2 in measurement circuit to have 2x gain
    }
        
    // TODO: Dont hardcode the voltage value used for 
    //       differentiating voltage ranges !
    // WARNING: Not relevant because we dont use multiple measurement ranges.
    //          However, it can be used as a poor mans nonlinear gain
    //          compensation over and above a certain value....
    // TODO: Remove this special handling ?
    if (abs(v) > 2300) {
      if (v>0) {
        v = v * V_CALIBRATION.getAdcGainCompPos2();
      } else {
        v = v * V_CALIBRATION.getAdcGainCompNeg2();
      }
    } else {
      if (v>0) {
        v = v * V_CALIBRATION.getAdcGainCompPos();
      } else {
        v = v * V_CALIBRATION.getAdcGainCompNeg();
      }
    }

    // crude adjustment, will differ between hardware
   
    // v=v * 0.9970;
    //v=v * 1.0255;
    //v =v* 0.991;
    
  }
  //DEBUG.print("Voltage nonlinear comp ");
  //DEBUG.print(v);
  v = V_CALIBRATION.adc_nonlinear_compensation(v);
  //DEBUG.print(" -> ");
  //DEBUG.println(v); 

  writeSamplingRate();  // update sampling rate here seems to work. Doing randomly other places often fails.... for some reason...
  writeShortSetting(); // update short setting as well
  writeRefInputSetting(); // update ref input

  v = v - V_CALIBRATION.nullValueVol[current_range];

  return v;
}



int ADCClass::dataReady() {
  return AD7176_dataReady();
}

void ADCClass::init() {

  setVrefMv(VREF_EXTERNAL_CAL); // set individualy...
  
  initADC();
  initDAC();

}


void ADCClass::initADC(){
  DEBUG.print("SETUP: ");
  AD7176_Reset();
  DEBUG.println(AD7176_Setup());
  DEBUG.println("REGS:");

  // copy of AD7176 registers
  //enum AD7176_registers regNr;

 // Gain_3
  for(int regNr = 0; regNr < Gain_3; ++regNr) {

    //if (regNr == ADC_Mode_Register) continue;
    //if (regNr == Interface_Mode_Register) continue;
    //if (regNr == Register_Check) continue;
    //if (regNr == Data_Register) continue;
    DEBUG.print("Write ");
    DEBUG.print(init_state[regNr].name);
    DEBUG.print(" ");
    DEBUG.print(regNr);
    DEBUG.print(" ");
    DEBUG.print(init_state[regNr].value, HEX);
    DEBUG.print(" ");
    DEBUG.print(AD7176_WriteRegister(init_state[regNr]));
    DEBUG.print(" bytes. Read ");
    DEBUG.print(init_state[regNr].name);
    DEBUG.print(" ");

    DEBUG.print(AD7176_ReadRegister(&AD7176_regs[regNr]));
    DEBUG.print(" bytes: ");
    DEBUG.println(AD7176_regs[regNr].value, HEX);
    // TODO: Find id another way. Does thus give the correct value ????
    if (regNr == 6) {
      deviceTypeId = AD7176_regs[regNr].value;
    }

    delay(10);
  }
  AD7176_UpdateSettings();

}


void ADCClass::initDAC(){
  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 3);  // initialising all channels to -10V - 10V range
  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x0); // init to 0;

  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, ADDRESS_DAC_ALL, 3);  // initialising all channels to -10V - 10V range
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, 0x6666); // init to some value;
}

bool ADCClass::use100uVSetResolution() {
  if (DAC_RANGE_LOW == -10.0) {
    return false;
  } else {
    return true;
  }
}

int64_t ADCClass::fltSetCommitVoltageSource(int64_t voltage_uV, bool dynamicRange) {
  operationType = SOURCE_VOLTAGE;
  setValue_micro = voltage_uV;

  float mv = voltage_uV / 1000.0;
  if (V_CALIBRATION.useDacCalibratedValues == true) {
    mv = V_CALIBRATION.dac_nonlinear_compensation(mv);
  }
   
  float dac_voltage = mv / 1000.0;  // DAC code operates with V instead of mV 
 
  dac_voltage = dac_voltage + V_CALIBRATION.getDacZeroComp();
  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board) {

  //  dac_voltage = dac_voltage * 0.994; // crude adjustment that will differ between hardware
  
    if (dac_voltage>0) {
      if (dac_voltage > 2.3) {
        dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompPos2();
      } else {
        dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompPos();
      }
    } else {
      if (dac_voltage < -2.3) {
        dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompNeg2();
      } else {
        dac_voltage = dac_voltage * V_CALIBRATION.getDacGainCompNeg();
      }
    }
    
    dac_voltage = dac_voltage * voltageInputDividerCompensation;

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

  }
  return setValue_micro;

}

 
 int64_t ADCClass::fltSetCommitCurrentSource(int64_t current_uA) {
   operationType = SOURCE_CURRENT;
  setValue_micro = current_uA;

  float mv = current_uA / 1000.0;

  if (C_CALIBRATION.useDacCalibratedValues == true) {
    //DEBUG.println("USE CALIBRATED VALUES FOR CURRENT");
    //DEBUG.print(mv,6);
    mv = C_CALIBRATION.dac_nonlinear_compensation(mv);
    //DEBUG.print(" adjusted to ");
    //DEBUG.println(mv,6);

  }
  float dac_voltage = mv / 1000.0;  // DAC code operates with V instead of mV    

  if (full_board) {
    if (current_range == AMP1) {
      dac_voltage = dac_voltage * 10.0; // with 1ohm shunt and x10 amplifier: 100mA is set by 1000mV
      dac_voltage = dac_voltage / 2.0; // if using 0.5ohm shunt instead of 1ohm shunt

      
      dac_voltage = dac_voltage - 0.00; // offset
      dac_voltage = dac_voltage + C_CALIBRATION.getDacZeroComp();

      dac_voltage = dac_voltage * 0.9; // TODO: DO above calculations with the mosfet resistance in account...
      if (dac_voltage < 0) { 
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg();
      } else {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos();
      }


      //dac_voltage = dac_voltage * 3.125; // after using 3 opamp diff amplifier before 1997-3...

 dac_voltage = dac_voltage *1.09;
      //DEBUG.println("Calculating current to set for 1A range");
      //DEBUG.flush();

    } else if (current_range == MILLIAMP10) {
      
      
      dac_voltage = dac_voltage * 1000.0; // with 100ohm shunt and x10 amplifier: 1mA range is set by 1000mV
      //TODO: Add daczerocomp also for 10mA range. Currently only imlemented for 1A range ?
      DEBUG.print("Setting:");
      DEBUG.print(dac_voltage);
      DEBUG.println("mV in DAC for 10mA range....");

      //dac_voltage = dac_voltage * 3.125; // after using 3 opamp diff amplifier before 1997-3...

      dac_voltage = dac_voltage + C_CALIBRATION.getDacZeroComp2();

      if (dac_voltage < 0) {
       dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg2(); 
      } else {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos2();
      }

      DEBUG.print("Adjusted to :");
      DEBUG.print(dac_voltage);
      DEBUG.println("mV in DAC for 10mA range....");
      
       
      DEBUG.println("Calculating current to set for 100mA range");
      DEBUG.flush();

    } else {
      DEBUG.print("ERROR: Unknown current range ");
      DEBUG.println(current_range);
      DEBUG.flush();

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
  
  // TODO: Find out what that means in steps for individual current ranges...
  // TODO: Add "auto" mode and "manual" mode ?
  
  if (abs(dac_voltage) <2.2 /*&& current_range == MILLIAMP10*/) {   // can move to 2.5 if reference voltage is 5v
    choice = 4;
    DAC_RANGE_LOW = -2.5;
    DAC_RANGE_HIGH = 2.5;
  }
  
  
  uint32_t span = (uint32_t)(choice << 2);

  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);

  bool serialOut = false;
  float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, serialOut);

  LTC2758_write(0, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  return setValue_micro;
 }
 
  
int64_t ADCClass::fltSetCommitVoltageLimit(int64_t voltage_uV, int8_t up_down_both) {

  setLimit_micro = voltage_uV;

  bool infoSerialOut = true;
  
  float mV = voltage_uV / 1000.0;
  
  if (infoSerialOut) {
    DEBUG.print("Set voltage limit to ");
    DEBUG.print(mV);
    DEBUG.println(" mV out from DAC");
  }

  // TODO: Use more adjustments for inaccuracies ? Can we share the ones that are used in souring voltage ?
  mV = mV * voltageInputDividerCompensation;
  //mV = mV -0.78 / 2.0; // TODO: Add limit offset to calibration store

  //SPAN 0 = 0 to +5V
  //     1 = 0 to +10V
  //     2 = -5 to +5 V
  //     3 = -10 to +10V
  //     4 = -2.5 to +2.5V
  //     5 = -2.5 to + 7.5V
  uint32_t choice = 1;
  float DAC_RANGE_LOW = 0.0;
  float DAC_RANGE_HIGH = 10.0;
  uint32_t span = (uint32_t)(choice << 2);

  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);
  float v_adj = LTC2758_voltage_to_code(mV/1000.0, DAC_RANGE_LOW, DAC_RANGE_HIGH, infoSerialOut);
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  return setLimit_micro;
 }
 
 int64_t ADCClass::fltSetCommitCurrentLimit(int64_t current_uA , int8_t up_down_both) {

  setLimit_micro = current_uA;

  bool infoSerialOut = true;

  float DAC_RANGE_LOW = 0.0;
  float DAC_RANGE_HIGH = 10.0;
  uint32_t choice = 1;
  uint32_t span = (uint32_t)(choice << 2);
     
  float dac_voltage;
  if (current_range == MILLIAMP10) {
    //TODO: use current shunt value for 10mA range
    //      In the code below it's hardcoded to fit to 100ohm !
    dac_voltage = current_uA/1000.0;// * 1000.0;
    if (infoSerialOut) {
      DEBUG.print("Set ");
      DEBUG.print(current_uA/1000.0, 3);
      DEBUG.print("mA current limit for 10mA range. That will set ");
      DEBUG.print(dac_voltage,6);
      DEBUG.println(" V out from DAC.");
    }
    if (dac_voltage > 10.0) {
      dac_voltage = 10.0;
      DEBUG.println("WARNING: To high limit value in 10mA range !");
    }
  } else {
    dac_voltage = current_uA/1000.0/1000.0;
    dac_voltage = 10.0* dac_voltage * (R_shunt_1A + R_mosfetSwitch);
    
    if (infoSerialOut) {
      DEBUG.print("Set ");
      DEBUG.print(current_uA/1000.0, 3);
      DEBUG.print("mA current limit for 1A range. That willl set ");
      DEBUG.print(dac_voltage,6);
      DEBUG.println(" V out from DAC.");
    }
    
    //dac_voltage = dac_voltage * 1.0685; // TODO: Should be part of initial crude calibration
    dac_voltage = dac_voltage *  V_CALIBRATION.getDacGainCompLim();
  }
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_SPAN_DAC, 0, span);
  float v_adj = LTC2758_voltage_to_code(dac_voltage, DAC_RANGE_LOW, DAC_RANGE_HIGH, infoSerialOut);
  LTC2758_write(1, LTC2758_CS, LTC2758_WRITE_CODE_UPDATE_DAC, 0, v_adj); 

  return setLimit_micro;
 }

 
 double ADCClass::measureCurrent(CURRENT_RANGE range){



    AD7176_ReadRegister(&AD7176_regs[4]);
    double v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
    v=v-vref*1000.0;

    //TODO: Change when testing with ADA4254!
    //v = v / 0.8;  // funnel amplifier x0.8
    v = v / 0.4;  // funnel amplifier x0.4 


    //v=v*10.0;

    float i = v;
    // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
    if (full_board == true) {
      if (range == MILLIAMP10) {
        i=v/R_shunt_10mA;
        //i=i*0.79;// apprx.... why? 
        //i=i/3.125; // After using two opamps in fromt of 1997-3....   Why did that give 3.125 gain ????


        if (i>0) {
          i = i * C_CALIBRATION.getAdcGainCompPos2();
 
        } else {
          i = i * C_CALIBRATION.getAdcGainCompNeg2();
        }

      } else {
 
      
         i=i/(R_shunt_1A + R_mosfetSwitch); 
        
         if (i>0) {
          i = i * C_CALIBRATION.getAdcGainCompPos();
         } else {
          i = i * C_CALIBRATION.getAdcGainCompNeg();
         }

        //i=i/3.125; // After using two opamps in fromt of 1997-3....   Why did that give 3.125 gain ????


      }
      i=i/10.0; // x10 amplifier
      //i = i / 32.0; // for testing with Ada4254

      // account for common mode error
      if (range == AMP1) {
       // i = i *   1.043; // 1ohm shunt, 1A range
        //i = i -V_FILTERS.mean* 0.000042; // account for common mode voltage giving wrong current (give too high result)
      } else {
       // i = i * 0.985; // 100ohm shunt, 10mA range
        //i = i -V_FILTERS.mean* 0.0000440; // account for common mode voltage giving wrong current (give too high result)      
      }
      writeSamplingRate();
      
      i=i - C_CALIBRATION.nullValueCur[range];
    

      
    }

    //DEBUG.print("Current nonlinear comp ");
    //DEBUG.print(i);
//    i = C_CALIBRATION.adc_nonlinear_compensation(i);
    //DEBUG.print(" -> ");
    //DEBUG.println(i);

    return i;
 }

 int64_t ADCClass::getSetValue_micro(){
  return setValue_micro;
 }

 int64_t ADCClass::getLimitValue_micro(){
  return setLimit_micro;
 }

 

    

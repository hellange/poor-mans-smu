
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
      AD7176_WriteRegister({0x28, 2, 0, 0x0204l}); // sinc3 
  } else if (value == 50000) {
      AD7176_WriteRegister({0x28, 2, 0, 0x0203l}); 
  } else {
    Serial.print("Illegal sample value:");
    Serial.println(value);
    Serial.flush();
  }
  
}



//TODO: GPIO is the how the io is implemented. Rename to what the function does
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
    if (operationType == SOURCE_VOLTAGE) {
      Serial.println("Switching current range (to A) while in voltage source must also update DAC output for limiting circuit.");
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
      Serial.println("Switching current range (to A) while in current source must also update DAC output");
      fltSetCommitCurrentSource(setValue_micro);
    }


  } else if (range == MILLIAMP10) {
   if (operationType == SOURCE_VOLTAGE) {
     Serial.println("Switching current range (to 10mA) while in voltage source must also update DAC output for limiting circuit.");
     fltSetCommitCurrentLimit(setLimit_micro, _SOURCE_AND_SINK);// printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
   } 
   else {
     Serial.println("Switching current range (to 10mA) while in current source must also update DAC output");
     fltSetCommitCurrentSource(setValue_micro);
   }
   digitalWrite(4, LOW);

  } else {
    Serial.println("ERROR: Unknown current range !!!");
    Serial.flush();
  }

}


double ADCClass::measureMilliVoltageRaw() {


  
  AD7176_ReadRegister(&AD7176_regs[4]);
     
  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR);

  
  v=v-VREF*1000.0;
  writeSamplingRate();  // update sampling rate here seems to work. Doing randomly other places often fails.... for some reason...

  return v;
}

bool ADCClass::hasCompliance() {
  return (digitalRead(2) == LOW or digitalRead(3) == LOW);
}

double ADCClass::measureMilliVoltage() {


  AD7176_ReadRegister(&AD7176_regs[4]);

  float v = (float) ((AD7176_regs[4].value*VFSR*1000.0)/FSR); 
  v=v-VREF*1000.0;

  //v = v / 0.8;  // funnel amplifiersetNullValue x0.8
  v = v / 0.4;  // funnel amplifier x0.4
  
  
  //v = v / 8.0; // gain
  
  // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
  if (full_board == true) {
    
    //v = v*2.0; // divide by 2 in measurement circuit
        
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
  //Serial.print("Voltage nonlinear comp ");
  //Serial.print(v);
  v = V_CALIBRATION.adc_nonlinear_compensation(v);
  //Serial.print(" -> ");
  //Serial.println(v); 

  writeSamplingRate();  // update sampling rate here seems to work. Doing randomly other places often fails.... for some reason...

  v=v - V_CALIBRATION.nullValueVol[current_range];

  return v;
}



int ADCClass::dataReady() {
  return AD7176_dataReady();
}

void ADCClass::init() {
  initADC();
  initDAC();
}

void ADCClass::initADC(){
  Serial.print("SETUP: ");
  AD7176_Reset();
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

    // There is a apprx. /2 on the sense input. This means that the voltage from DAC must be half of the expected output
    //float voltageInputDividerCompensation = 0.5; 
    
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
    
    //dac_voltage = dac_voltage * voltageInputDividerCompensation;

    

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
    //Serial.println("USE CALIBRATED VALUES FOR CURRENT");
    //Serial.print(mv,6);
    mv = C_CALIBRATION.dac_nonlinear_compensation(mv);
    //Serial.print(" adjusted to ");
    //Serial.println(mv,6);

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


      //dac_voltage = dac_voltage * 3.125; // after using 3 opamp diff amplifier before 1997-3.... hmm...

 dac_voltage = dac_voltage *1.09;
      //Serial.println("Calculating current to set for 1A range");
      //Serial.flush();

    } else if (current_range == MILLIAMP10) {
      
      
      dac_voltage = dac_voltage * 1000.0; // with 100ohm shunt and x10 amplifier: 1mA range is set by 1000mV
      //TODO: Add daczerocomp also for 10mA range. Currently only imlemented for 1A range ?
      Serial.print("Setting:");
      Serial.print(dac_voltage);
      Serial.println("mV in DAC for 10mA range....");

      //dac_voltage = dac_voltage * 0.960;

      //dac_voltage = dac_voltage * 3.125; // after using 3 opamp diff amplifier before 1997-3.... hmm...
 //dac_voltage = dac_voltage /1.015;

      dac_voltage = dac_voltage + C_CALIBRATION.getDacZeroComp2();

      if (dac_voltage < 0) {
       dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompNeg2(); 
      } else {
        dac_voltage = dac_voltage * C_CALIBRATION.getDacGainCompPos2();
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
    Serial.print("Set voltage limit to ");
    Serial.print(mV);
    Serial.println(" mV out from DAC");
  }

  // TODO: Use more adjustments for inaccuracies ? Can we share the ones that are used in souring voltage ?
  mV = mV / 2.0; // divide by two in voltage measurement circuit
  
  mV = mV * 1.02; // TODO: Should be part of initial crude calibration 
  mV = mV -0.78 / 2.0; // TODO: Add limit offset to calibration store

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

  bool infoSerialOut = false;

  float DAC_RANGE_LOW = 0.0;
  float DAC_RANGE_HIGH = 10.0;
  uint32_t choice = 1;
  uint32_t span = (uint32_t)(choice << 2);
     
  float dac_voltage;
  if (current_range == MILLIAMP10) {
    dac_voltage = current_uA/1000.0;// * 1000.0;
    if (infoSerialOut) {
      Serial.print("Set ");
      Serial.print(current_uA/1000.0, 3);
      Serial.print("mA current limit for 10mA range. That will set ");
      Serial.print(dac_voltage,6);
      Serial.println(" V out from DAC.");
    }
    if (dac_voltage > 10.0) {
      dac_voltage = 10.0;
      Serial.println("WARNING: To high limit value in 10mA range !");
    }
  } else {
    dac_voltage = current_uA/1000.0/1000.0;
    dac_voltage = 10.0* dac_voltage * (R_shunt_1A + R_mosfetSwitch);
    
    if (infoSerialOut) {
      Serial.print("Set ");
      Serial.print(current_uA/1000.0, 3);
      Serial.print("mA current limit for 1A range. That willl set ");
      Serial.print(dac_voltage,6);
      Serial.println(" V out from DAC.");
    }
    
    dac_voltage = dac_voltage * 1.0685; // TODO: Should be part of initial crude calibration
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
    v=v-VREF*1000.0;

    //TODO: Change when testing with ADA4254!
    //v = v / 0.8;  // funnel amplifier x0.8
    v = v / 0.4;  // funnel amplifier x0.4 


    //v=v*10.0;

    float i = v;
    // DONT INCLUDE THESE ADJUSTMENTS WHEN TESTING ONLY DAC/ADC BOARD !!!!
    if (full_board == true) {
      if (range == MILLIAMP10) {
        i=v/100.0; // 100 ohm shunt.
        //i=i*0.79;// apprx.... why? 
        //i=i/3.125; // After using two opamps in fromt of 1997-3....   Why did that give 3.125 gain ????


        if (i>0) {
          i = i * C_CALIBRATION.getAdcGainCompPos2();
          i=i*1.012;

        } else {
          i = i * C_CALIBRATION.getAdcGainCompNeg2();
          i = i *1.01;
        }

      } else {
        //i=i/1.04600; // 1ohm shunt + resistance in range switch mosfet
        i=i*5.0; // if 0.5 ohm shunt instead of ohm shunt;

      
      i=i/(R_shunt_1A + R_mosfetSwitch); 
        
        //i=i*0.79; // apprx.... why ?
        if (i>0) {
          i = i * C_CALIBRATION.getAdcGainCompPos();
          //i = i * 0.002;
        } else {
          i = i * C_CALIBRATION.getAdcGainCompNeg();
          i = i *0.98;
        }

              //i=i/3.125; // After using two opamps in fromt of 1997-3....   Why did that give 3.125 gain ????

      }
      //i=i/10.0; // x10 amplifier
      i = i / 32.0;

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

    return i;
 }

 int64_t ADCClass::getSetValue_micro(){
  return setValue_micro;
 }

 int64_t ADCClass::getLimitValue_micro(){
  return setLimit_micro;
 }

 

    

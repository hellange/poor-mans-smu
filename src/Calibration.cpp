#include "Calibration.h"
#include "Stats.h"
#include "Arduino.h"
#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include <EEPROM.h>
#include "eeprom_adr.h"
#include "Filters.h"
#include "digit_util.h"
#include "SMU_HAL_717x.h"
#include "Debug.h"
#include "EEPROMAnything.h"

CalibrationClass V_CALIBRATION;
CalibrationClass C_CALIBRATION;

//extern ADCClass SMU[];
  
  void float_array_init(float *a, const int ct, ...) {
  va_list args;
  va_start(args, ct);
  for(int i = 0; i < ct; ++i) {
    double  d = (float)va_arg(args,double);
    a[i] = d;
    DEBUG.print(d,2);
    DEBUG.print("(");
    DEBUG.print(a[i],2);
    DEBUG.print(")");

    if (i == ct-1) {
      DEBUG.print(",");
    }
  }
  DEBUG.println("");
  va_end(args);
}

// using static to share among instances. TODO: Fix this mess
float CalibrationClass::nullValueVol[2];
float CalibrationClass::nullValueCur[2];

void CalibrationClass::init(SMU_HAL &SMU,OPERATION_TYPE operationType_) {
  SMU1 = &SMU;
  operationType = operationType_;

  if (operationType == SOURCE_VOLTAGE) {
    ea_dac_zero_comp = EA_DAC_ZERO_COMP_VOL;
    ea_dac_zero_comp2 = EA_DAC_ZERO_COMP_VOL2;

  } else { 
    ea_dac_zero_comp = EA_DAC_ZERO_COMP_CUR;
    ea_dac_zero_comp2 = EA_DAC_ZERO_COMP_CUR2;
  }

  ea_adc_zero_comp_vol = EA_ADC_ZERO_COMP_VOL;
  ea_adc_zero_comp_vol2 = EA_ADC_ZERO_COMP_VOL2;

  ea_adc_zero_comp_cur = EA_ADC_ZERO_COMP_CUR;
  ea_adc_zero_comp_cur2 = EA_ADC_ZERO_COMP_CUR2;

  
  if (operationType == SOURCE_VOLTAGE) {
    // set value
//    adc_cal_points = 37;
//    DEBUG.println("Voltage measurement nonlinearity:");
//    float_array_init(set_adc, adc_cal_points,  -12000.00, -11000.00, -10000.00, -9000.00, -8000.00, -7000.00, -6000.00, -5000.00, -4000.00, -3500.00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, -50.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 2000.00, 2500.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00, 12000.00);
//    float_array_init(meas_adc, adc_cal_points, -11992.94, -10996.74, -10000.00, -8999.41, -7998.84, -6998.41, -5996.70, -4995.87, -3996.70, -3497.11, -2997.69, -2497.78, -1997.91, -0999.54, -499.96, -100.01, -50.00, 0.00, 50.06, 100.16, 250.34, 400.54, 499.94, 899.69, 0999.60, 1997.99, 2497.91, 2997.80, 3997.05, 4996.08, 5996.35, 6999.66, 8000.00, 8994.87,  9985.00, 10977.66, 11986.00);
//
//    dac_cal_points = 26;
//    DEBUG.println("Voltage source nonlinearty:");
//    float_array_init(set_dac, dac_cal_points,  -11000.00, -10000.00, -5000.00, -4000.00, -3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00);
//    float_array_init(meas_dac, dac_cal_points, -11000.00, -10000.00, -5000.00, -3999.98, -2999.90, -2000.14, -1000.07, 0.00, 5.00, 49.99, 100.02, 500.00, 0999.98, 1999.97, 3000.00, 3500.20, 4000.08, 4200.06, 4500.05, 5000.04, 6000.01, 7000.17, 8000.12, 9000.10, 10000.00, 11000.00);
  
    ea_dac_gain_comp_pos = EA_DAC_GAIN_COMP_POS_VOL;
    ea_dac_gain_comp_neg = EA_DAC_GAIN_COMP_NEG_VOL;

    ea_dac_gain_comp_pos2 = EA_DAC_GAIN_COMP_POS_VOL2;
    ea_dac_gain_comp_neg2 = EA_DAC_GAIN_COMP_NEG_VOL2;


    // TODO DIffer between VOLTAGE and CURRENT mode for limit !
    ea_dac_gain_comp_lim = EA_DAC_GAIN_COMP_POS_LIM;

    
    ea_adc_gain_comp_pos = EA_ADC_GAIN_COMP_POS_VOL;
    ea_adc_gain_comp_neg = EA_ADC_GAIN_COMP_NEG_VOL;

    ea_adc_gain_comp_pos2 = EA_ADC_GAIN_COMP_POS_VOL2;
    ea_adc_gain_comp_neg2 = EA_ADC_GAIN_COMP_NEG_VOL2;

    ea_adc_nonlinear_comp_nr = EA_ADC_NONLINEAR_COMP_NR_VOL;
    ea_adc_nonlinear_comp_start = EA_ADC_NONLINEAR_COMP_START_VOL;

    FILTERS = &V_FILTERS;
    DEBUG.println("");
  DEBUG.println(" Voltage source calibration data");
  DEBUG.println(" ==================================================");

    readAdcCalFromEeprom();

  } else {

        // set value
//    adc_cal_points = 15;
//    DEBUG.println("Current measurement nonlinearity:");
//    float_array_init(set_adc, adc_cal_points, -500.00, -400.00, -300.00, -200.00, -100.00, -50.00, -10.00, 0.00, 10.00, 50.00, 100.00, 200.00, 300.00, 400.00, 500.00 );
//    float_array_init(meas_adc, adc_cal_points,-500.00, -400.17, -300.09, -199.93, -099.96, -50.00, -10.00, 0.00, 10.02, 50.10, 100.18, 200.36, 300.69, 400.88, 500.00 );
//
//    dac_cal_points = 15;
//    DEBUG.println("Current source nonlinearity:");
//    float_array_init(set_dac, dac_cal_points,   -500.00, -400.00, -300.00, -200.00, -100.00, -50.00, -10.000, 0.00, 10.00, 50.00, 100.00, 200.00, 300.00, 400.00, 500.00);
//    float_array_init(meas_dac, dac_cal_points,  -500.00, -399.94, -299.90, -199.93, -099.94, -49.92, -10.006, 0.00, 10.00, 49.99, 099.96, 199.93, 299.97, 399.94, 500.00);

    ea_dac_gain_comp_pos = EA_DAC_GAIN_COMP_POS_CUR;
    ea_dac_gain_comp_neg = EA_DAC_GAIN_COMP_NEG_CUR;

    ea_dac_gain_comp_pos2 = EA_DAC_GAIN_COMP_POS_CUR2;
    ea_dac_gain_comp_neg2 = EA_DAC_GAIN_COMP_NEG_CUR2;

    ea_adc_gain_comp_pos2 = EA_ADC_GAIN_COMP_POS_CUR2;
    ea_adc_gain_comp_neg2 = EA_ADC_GAIN_COMP_NEG_CUR2;

    ea_adc_gain_comp_pos = EA_ADC_GAIN_COMP_POS_CUR;
    ea_adc_gain_comp_neg = EA_ADC_GAIN_COMP_NEG_CUR;

   // TODO ADD limit when in current mode, ref also comment about EA_DAC_GAIN_COMP_POS_LIM!
    //xxxxea_dac_gain_comp_lim = XXXXEA_DAC_GAIN_COMP_POS_LIM;

    ea_adc_nonlinear_comp_nr = EA_ADC_NONLINEAR_COMP_NR_CUR;
    ea_adc_nonlinear_comp_start = EA_ADC_NONLINEAR_COMP_START_CUR;

    FILTERS = &C_FILTERS;
        DEBUG.println("");

  DEBUG.println(" Current source calibration data");
  DEBUG.println(" ==================================================");

    readAdcCalFromEeprom();

  }

  nullValueVol[0] = 0.0;
  nullValueVol[1] = 0.0;
  nullValueCur[0] = 0.0;
  nullValueCur[1] = 0.0;
  timeSinceLastChange = millis();
  
  // DAC gain compensation adjust
   DEBUG.println(" READING COMPENSATION ENTRIES FROM EEPROM");

  dacGainCompPos = floatFromEeprom(ea_dac_gain_comp_pos);
  DEBUG.print("Read dacGainCompPos from eeprom address ");
  DEBUG.print(ea_dac_gain_comp_pos,HEX);
  DEBUG.print(": ");
  if (isnan(dacGainCompPos)) {
    DEBUG.print("Not defined. Write default value:");
    dacGainCompPos = 1.0;
    floatToEeprom(ea_dac_gain_comp_pos,dacGainCompPos); // write initial default
  }
  DEBUG.println(dacGainCompPos,7);

  dacGainCompPos2 = floatFromEeprom(ea_dac_gain_comp_pos2);
  DEBUG.print("Read dacGainCompPos2 from eeprom address ");
  DEBUG.print(ea_dac_gain_comp_pos2,HEX);
  DEBUG.print(": ");
  if (isnan(dacGainCompPos2)) {
    DEBUG.print("Not defined. Write default value:");
    dacGainCompPos2 = 1.0;
    floatToEeprom(ea_dac_gain_comp_pos2,dacGainCompPos2); // write initial default
  }
  DEBUG.println(dacGainCompPos2,7);

 
  dacGainCompNeg = floatFromEeprom(ea_dac_gain_comp_neg);
  DEBUG.print("Read dacGainCompNeg from eeprom address ");
  DEBUG.print(ea_dac_gain_comp_neg,HEX);
  DEBUG.print(": ");
  if (isnan(dacGainCompNeg)) {
    DEBUG.print("Not defined. Write default value:");
    dacGainCompNeg = 1.0;
    floatToEeprom(ea_dac_gain_comp_neg,dacGainCompNeg); // write initial default
  }
  DEBUG.println(dacGainCompNeg,7);

 dacGainCompNeg2 = floatFromEeprom(ea_dac_gain_comp_neg2);
  DEBUG.print("Read dacGainCompNeg2 from eeprom address ");
  DEBUG.print(ea_dac_gain_comp_neg2,HEX);
  DEBUG.print(": ");
  if (isnan(dacGainCompNeg2)) {
    DEBUG.print("Not defined. Write default value:");
    dacGainCompNeg2 = 1.0;
    floatToEeprom(ea_dac_gain_comp_neg2,dacGainCompNeg2); // write initial default
  }
  DEBUG.println(dacGainCompNeg2,7);

  adcGainCompPos2 = floatFromEeprom(ea_adc_gain_comp_pos2);
      DEBUG.print("Read adcGainCompPos2 from eeprom address ");
      DEBUG.print(ea_adc_gain_comp_pos2,HEX);
      DEBUG.print(": ");
      if (isnan(adcGainCompPos2)) {
        DEBUG.print("Not defined. Write default value:");
        adcGainCompPos2 = 1.0;
        floatToEeprom(ea_adc_gain_comp_pos2,adcGainCompPos2); // write initial default
      }
      DEBUG.println(adcGainCompPos2,7);

   adcGainCompNeg2 = floatFromEeprom(ea_adc_gain_comp_neg2);
    DEBUG.print("Read adcGainCompNeg2 from eeprom address ");
    DEBUG.print(ea_adc_gain_comp_neg2,HEX);
    DEBUG.print(": ");
    if (isnan(adcGainCompNeg2)) {
      DEBUG.print("Not defined. Write default value:");
      adcGainCompNeg2 = 1.0;
      floatToEeprom(ea_adc_gain_comp_neg2,adcGainCompNeg2); // write initial default
    }
    DEBUG.println(adcGainCompNeg2,7);



 dacGainCompLim = floatFromEeprom(ea_dac_gain_comp_lim);
  DEBUG.print("Read dacGainCompLim from eeprom address ");
  DEBUG.print(ea_dac_gain_comp_lim,HEX);
  DEBUG.print(": ");
  if (isnan(dacGainCompLim)) {
    DEBUG.print("Not defined. Write default value:");
    dacGainCompLim = 1.0;
    floatToEeprom(ea_dac_gain_comp_lim,dacGainCompLim); // write initial default
  }
  DEBUG.println(dacGainCompLim,7);

  

 adcGainCompPos = floatFromEeprom(ea_adc_gain_comp_pos);
 DEBUG.print("Read adcGainCompPos from eeprom address ");
  DEBUG.print(ea_adc_gain_comp_pos,HEX);
  DEBUG.print(": ");  
  if (isnan(adcGainCompPos)) {
    DEBUG.print("Not defined. Write default value:");
    adcGainCompPos = 1.0;
    floatToEeprom(ea_adc_gain_comp_pos,adcGainCompPos); // write initial default
  }
  DEBUG.println(adcGainCompPos,7);
 
  adcGainCompNeg = floatFromEeprom(ea_adc_gain_comp_neg);
 DEBUG.print("Read adcGainCompNeg from eeprom address ");
  DEBUG.print(ea_adc_gain_comp_neg,HEX);
  DEBUG.print(": ");  
  if (isnan(adcGainCompNeg)) {
    DEBUG.print("Not defined. Write default value:");
    adcGainCompNeg = 1.0;
    floatToEeprom(ea_adc_gain_comp_neg,adcGainCompNeg); // write initial default
  }
  DEBUG.println(adcGainCompNeg,7);

 dacZeroComp = floatFromEeprom(ea_dac_zero_comp);
 DEBUG.print("Read dacZeroComp from eeprom address ");
  DEBUG.print(ea_dac_zero_comp,HEX);
  DEBUG.print(": ");
  if (isnan(dacZeroComp)) {
    DEBUG.print("Not defined. Write default value:");
    dacZeroComp = 0.0; // use millivolt
    floatToEeprom(ea_dac_zero_comp,dacZeroComp); // write initial default
  } else if (abs(dacZeroComp) < -10000.0 or abs(dacZeroComp) > 10000.0) {
    DEBUG.print("WARNING: Suspect dac zero value:");
    DEBUG.println(dacZeroComp);
    dacZeroComp = 0.0;
    DEBUG.print("Setting dac zero to:");
    DEBUG.println(dacZeroComp);
    
  }
  DEBUG.println(dacZeroComp,7);


  dacZeroComp2 = floatFromEeprom(ea_dac_zero_comp2);
 DEBUG.print("Read dacZeroComp2 from eeprom address ");
  DEBUG.print(ea_dac_zero_comp2,HEX);
  DEBUG.print(": ");
  if (isnan(dacZeroComp2)) {
    DEBUG.print("Not defined. Write default value:");
    dacZeroComp2 = 0.0; // use millivolt
    floatToEeprom(ea_dac_zero_comp2,dacZeroComp2); // write initial default
  } else if (abs(dacZeroComp2) < -10000.0 or abs(dacZeroComp2) > 10000.0) {
    DEBUG.print("WARNING: Suspect dac zero value:");
    DEBUG.println(dacZeroComp2);
    dacZeroComp2 = 0.0;
    DEBUG.print("Setting dac zero to:");
    DEBUG.println(dacZeroComp2);
    
  }
  DEBUG.println(dacZeroComp2,7);






 float adcZeroCompVol = floatFromEeprom(ea_adc_zero_comp_vol);
  
 DEBUG.print("Read adcZeroCompVol from eeprom address ");
  DEBUG.print(ea_adc_zero_comp_vol,HEX);
  DEBUG.print(": ");
  if (isnan(adcZeroCompVol)) {
    DEBUG.print("Not defined. Write default value:");
    adcZeroCompVol = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_vol,adcZeroCompVol); // write initial default
  } else if (abs(adcZeroCompVol) < -10000.0 or abs(adcZeroCompVol) > 10000.0) {
    DEBUG.print("WARNING: Suspect adc zero value:");
    DEBUG.println(adcZeroCompVol);
    adcZeroCompVol = 0.0;
    DEBUG.print("Setting adc zero to:");
    DEBUG.println(adcZeroCompVol);
  }
  DEBUG.println(adcZeroCompVol,7);
  nullValueVol[0] = adcZeroCompVol;

  float adcZeroCompVol2 = floatFromEeprom(ea_adc_zero_comp_vol2);
  DEBUG.print("Read adcZeroCompVol2 from eeprom address ");
  DEBUG.print(ea_adc_zero_comp_vol2,HEX);
  DEBUG.print(": ");
  if (isnan(adcZeroCompVol2)) {
    DEBUG.print("Not defined. Write default value:");
    adcZeroCompVol2 = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_vol2,adcZeroCompVol2); // write initial default
  } else if (abs(adcZeroCompVol2) < -10000.0 or abs(adcZeroCompVol2) > 10000.0) {
    DEBUG.print("WARNING: Suspect adc zero value:");
    DEBUG.println(adcZeroCompVol2);
    adcZeroCompVol2 = 0.0;
    DEBUG.print("Setting adc zero to:");
    DEBUG.println(adcZeroCompVol2);
  }
  DEBUG.println(adcZeroCompVol2,7);
  nullValueVol[1] = adcZeroCompVol2;


   float adcZeroCompCur = floatFromEeprom(ea_adc_zero_comp_cur);
   float adcZeroCompCur2 = floatFromEeprom(ea_adc_zero_comp_cur2);
  
  DEBUG.print("Read adcZeroCompCur from eeprom address ");
  DEBUG.print(ea_adc_zero_comp_cur,HEX);
  DEBUG.print(": ");
  if (isnan(adcZeroCompCur)) {
    DEBUG.print("Not defined. Write default value:");
    adcZeroCompCur = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_cur, adcZeroCompCur); // write initial default
  } else if (abs(adcZeroCompCur) < -10000.0 or abs(adcZeroCompCur) > 10000.0) {
    DEBUG.print("WARNING: Suspect adc zero value:");
    DEBUG.println(adcZeroCompCur);
    adcZeroCompCur = 0.0;
    DEBUG.print("Setting adc zero to:");
    DEBUG.println(adcZeroCompCur);
  }
  DEBUG.println(adcZeroCompCur,7);
  //TODO: Differ between the two null value (ranges)
  nullValueCur[0] = adcZeroCompCur;




 DEBUG.print("Read adcZeroCompCur2 from eeprom address ");
  DEBUG.print(ea_adc_zero_comp_cur2,HEX);
  DEBUG.print(": ");
  if (isnan(adcZeroCompCur2)) {
    DEBUG.print("Not defined. Write default value:");
    adcZeroCompCur2 = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_cur2, adcZeroCompCur2); // write initial default
  } else if (abs(adcZeroCompCur2) < -10000.0 or abs(adcZeroCompCur2) > 10000.0) {
    DEBUG.print("WARNING: Suspect adc zero value:");
    DEBUG.println(adcZeroCompCur2);
    adcZeroCompCur = 0.0;
    DEBUG.print("Setting adc zero to:");
    DEBUG.println(adcZeroCompCur2);
  }
  DEBUG.println(adcZeroCompCur2,7);
  nullValueCur[1] = adcZeroCompCur2;


}


int autoCalDacTimer = millis();
float autoCalV = -10000.0;
bool autoCalDone1 = false;
int autoCalArrayPointer = 0;
 // should be stored in these:
 //float set_adc[100]; 
 //float meas_adc[100];
 //int adc_cal_points=0;
 float maxValueCal = 10000.00;
 float minValueCal = -10000.00;
 float stepValueCal = 1000.00;
 
void CalibrationClass::startAutoCal() {
  // use different values for current
  if (operationType == SOURCE_CURRENT) {
    maxValueCal = 400.0;
    minValueCal = -400.0;
    stepValueCal = 25.0;
  }
  
  if (!autoCalInProgress) {
    DEBUG.print("STARTING AUTO CALIBRATION OF ADC FOR ");
    DEBUG.println(operationType == SOURCE_CURRENT ? "CURRENT SOURCE" : "VOLTAGE SOURCE");
    autoCalInProgress = true;
    autoCalDacTimer = millis();
    autoCalV = minValueCal;
    useCalibratedValues = false;
    autoCalArrayPointer = 0;
    SMU1->setSamplingRate(20);
    FILTERS->setFilterSize(5);

    // clear existing cals from ram
    int calPtr = 0;
    for (float v = minValueCal; v <= maxValueCal; v=v+stepValueCal) {
      //set_adc[calPtr] = v;
      //meas_adc[calPtr] = v;
      calPtr ++;
    }
    adc_cal_points = calPtr;
    
  }
}

void CalibrationClass::readAdcCalFromEeprom() {
  DEBUG.println(" READING NONLINEAR CALIBRATION ENTRIES FROM EEPROM");
  int dataPtr = 0;
  float nr = floatFromEeprom(ea_adc_nonlinear_comp_nr);
  if (isnan(nr)) {
    DEBUG.println("WARNING: No calibration data found in eeprom. Use default values...");
    return;
  }
  if (nr < 10.0 or nr > 1000.0) {
    DEBUG.print("WARNING: Suspect data. Expecting a reasonable number of calibration entries. Not ");
    DEBUG.println((int)nr);
    return;
  }
 
  int nrOfPoints = (int)nr;
  DEBUG.print("Expecting ");
  DEBUG.print(nrOfPoints);
  DEBUG.println(" calibration entries in eeprom!");
  adc_cal_points = nrOfPoints;
  
  for (int adr = ea_adc_nonlinear_comp_start; adr < ea_adc_nonlinear_comp_start + nrOfPoints*8; adr = adr + 8) {
    float set = floatFromEeprom(adr);
    float meas = floatFromEeprom(adr+4);
    if (isnan(set) or isnan(meas)) {
      DEBUG.println("ERROR: No ADC nonlinear calibration data found!");
      DEBUG.print("       Current set value:");
      DEBUG.println(set);
      break;
    }
    set_adc[dataPtr] = set;
    meas_adc[dataPtr] = meas;
    DEBUG.print("Address:");
    DEBUG.print(adr);
    DEBUG.print(" READ set:");
    DEBUG.print(set,3);
    DEBUG.print(", meas:");
    DEBUG.println(meas,3);
    dataPtr ++;
  }
  DEBUG.print("Read ");
  DEBUG.print(nrOfPoints);
  DEBUG.print(" calibration entries from address ");
  DEBUG.print(ea_adc_nonlinear_comp_start);
  DEBUG.print(" to ");
  DEBUG.println(ea_adc_nonlinear_comp_start + nrOfPoints*8);
  useCalibratedValues = true;

   //this->saveGain(10);
     this->readGain();

  }

void CalibrationClass::saveGain(double value) {
  config_gain g;
  g.gain_negative = -value;
  g.gain_positive = value;
  EEPROM_writeAnything(0, g);
  DEBUG.println("saved gain ");

}

CalibrationClass::config_gain CalibrationClass::readGain()  {
  config_gain g;
    DEBUG.println("read gain ");

   EEPROM_readAnything(0, g);
      DEBUG.println(g.gain_negative);
            DEBUG.println(g.gain_positive);
  return g;

}

void CalibrationClass::writeAdcCalToEeprom(int nrOfPoints) {
  int dataPtr = 0;
  floatToEeprom(ea_adc_nonlinear_comp_nr, (float)nrOfPoints);
  for (int adr = ea_adc_nonlinear_comp_start; adr <= ea_adc_nonlinear_comp_start + nrOfPoints*8; adr = adr + 8) {
    floatToEeprom(adr, set_adc[dataPtr]);
    floatToEeprom(adr+4, meas_adc[dataPtr]);
    DEBUG.print("Address:");
    DEBUG.print(adr);
    DEBUG.print(" WRITE set:");
    DEBUG.print(set_adc[dataPtr],3);
    DEBUG.print(", meas:");
    DEBUG.println(meas_adc[dataPtr],3);
    dataPtr ++;
  }
  DEBUG.print("Wrote ");
  DEBUG.print(nrOfPoints);
  DEBUG.print(" calibration entries from address ");
  DEBUG.print(ea_adc_nonlinear_comp_start);
  DEBUG.print(" to ");
  DEBUG.println(ea_adc_nonlinear_comp_start + adc_cal_points*8);
  
}


void CalibrationClass::autoCalADCfromDAC() {
  if (autoCalInProgress == false) {
    return;
  } 


  if (!autoCalDone1 && autoCalDacTimer + 500 < (int)millis()) {
    DEBUG.print("Set value to:");
    DEBUG.println(autoCalV);
    GD.__end();
    if (operationType == SOURCE_VOLTAGE) {
       SMU1->fltSetCommitVoltageSource(autoCalV*1000.0, true);
    } else {
       SMU1->fltSetCommitCurrentSource(autoCalV*1000.0);
    }
    GD.resume();

    set_adc[autoCalArrayPointer] = autoCalV;
    DEBUG.print("Set ");
    DEBUG.print(autoCalV);
    autoCalDone1 = true;
  }
  if (autoCalDacTimer + 5000 < (int)millis()) {

    float v = FILTERS->mean;
  
    DEBUG.print(" > ");
    DEBUG.print(v);
    DEBUG.print(" (filter id:");
    DEBUG.print(FILTERS->id);
    DEBUG.println(")");


    DEBUG.print("Changed cal:");
    DEBUG.print(meas_adc[autoCalArrayPointer]);
    meas_adc[autoCalArrayPointer] = v;
    DEBUG.print(" to ");
    DEBUG.println(meas_adc[autoCalArrayPointer]);

    
     autoCalV += stepValueCal;
     autoCalArrayPointer ++;
    if (autoCalV > maxValueCal) {
      autoCalInProgress = false;
      autoCalDone = true;
      useCalibratedValues = true;
      adc_cal_points = autoCalArrayPointer;
      FILTERS->setFilterSize(5);
      DEBUG.print("DONE nonlinear calibration of ");
      DEBUG.print(adc_cal_points);
      DEBUG.println(" points");
      writeAdcCalToEeprom(adc_cal_points);
    } else {
      autoCalDacTimer = millis();
      autoCalDone1 = false;
      
    }
    
  }
 

  
}


bool CalibrationClass::toggleCalibratedValues() {
  if (timeSinceLastChange + 100 > millis()){
    return useCalibratedValues;
  }
  if (useCalibratedValues == true) {
    useCalibratedValues = false;
  } else  {
    useCalibratedValues = true;
  }
  timeSinceLastChange = millis();

  return useCalibratedValues;
}

bool CalibrationClass::nullValueIsSet(CURRENT_RANGE current_range) {
  return nullValueVol[current_range] != 0.0 or nullValueCur[current_range] != 0.0;
}

bool CalibrationClass::relativeValueIsSet(CURRENT_RANGE current_range) {
  return relativeValue[current_range] != 0.0;
}

void CalibrationClass::setNullValueVol(float v, CURRENT_RANGE current_range) {
  nullValueVol[current_range] = v;
  if (current_range == 0) {
    floatToEeprom(ea_adc_zero_comp_vol, v);
  } else {
    floatToEeprom(ea_adc_zero_comp_vol2, v);
  }
}

void CalibrationClass::setNullValueCur(float v, CURRENT_RANGE current_range) {
  nullValueCur[current_range] = v;
  //TODO: Store different ranges in different locations
  if (current_range == 0) {
    floatToEeprom(ea_adc_zero_comp_cur, v);
  } else {
    floatToEeprom(ea_adc_zero_comp_cur2, v);
  }
}


void CalibrationClass::toggleRelativeValue(float v, CURRENT_RANGE current_range) {
  if (timeSinceLastChange + 100 > millis()){
    return;
  }
  if (relativeValue[current_range] != 0.0) {
    relativeValue[current_range] = 0.0;
  } else {
    relativeValue[current_range] = v;
  } 
  DEBUG.print("setRelativeValue to:");
  DEBUG.println(v);
  timeSinceLastChange = millis();

}
// todo: change parameter name to mv ?
float CalibrationClass::adc_nonlinear_compensation(float v){

  if (!useCalibratedValues) {
    //DEBUG.println("Dont use adc calibrated values");
    return v;
  } else {
    //DEBUG.println("Use calibrated values");
  }
  
  // Nonlinearity
  for (int i=0;i<adc_cal_points -1; i++) {
   // if (v > meas_adc[i] && v <= meas_adc[i+1]) {
    if (v > set_adc[i] && v <= set_adc[i+1]) {
//      DEBUG.print("Found within range ");
//      DEBUG.print(set_adc[i]);
//      DEBUG.print(": ");
//      DEBUG.println(set_adc[i+1]);
      float adj_factor_low = set_adc[i] - meas_adc[i];
      float adj_factor_high = set_adc[i+1] - meas_adc[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_adc[i+1] - set_adc[i];
      //float rangeStart = set_adc[i];
      //float rangeEnd = set_adc[i+1];
      float partWithinRange = ( (v-set_adc[i]) / range); // 0 to 1. Where then 0.5 is in the middle of the range 
      if (partWithinRange < 0.0) {
        DEBUG.println("ERROR: calibration partWithinRange gave negative value...");
      }
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;
//      DEBUG.print("meas:");  
//      DEBUG.print(v, 3);
//      DEBUG.print(", range:");  
//      DEBUG.print(range, 3);
//      DEBUG.print(", part:");  
//      DEBUG.print(partWithinRange, 3);
//      DEBUG.print(", factor:");  
//      DEBUG.println(adj_factor, 3);
//
//      DEBUG.flush();
       
      v= v + adj_factor;
      return v;
    }
  }
  return v;
}



float CalibrationClass::dac_nonlinear_compensation(float milliVolt) {
  // Nonlinearity
  //DEBUG.print("Looking up in DAC comp table for ");
  //DEBUG.print(milliVolt);
  //DEBUG.println(" millivolt");
  float v = milliVolt;
  for (int i=0;i<dac_cal_points -1;i++) {
    //if (v > meas_dac[i] && v <= meas_dac[i+1]) {
      if (v > set_dac[i] && v <= set_dac[i+1]) {
      float adj_factor_low = set_dac[i] - meas_dac[i];
      float adj_factor_high = set_dac[i+1] - meas_dac[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_dac[i+1] - set_dac[i];
      float partWithinRange = ( (v-set_dac[i]) / range); /* 0 to 1. Where then 0.5 is in the middle of the range */
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;

//      DEBUG.print("meas:");  
//      DEBUG.print(v, 4);
//      DEBUG.print(", range:");  
//      DEBUG.print(range, 4);
//      DEBUG.print(", part:");  
//      DEBUG.print(partWithinRange, 4);
//      DEBUG.print(", diff:");  
//      DEBUG.print(adj_factor_diff, 4);
//      DEBUG.print(", factor:");  
//      DEBUG.println(adj_factor, 4);
//      DEBUG.flush();
      v = v + adj_factor; 
      
      return v;
    }
  } 
  return milliVolt;  
}

void CalibrationClass::floatToEeprom(int address, float f) {
  DEBUG.print("Trying to convert float:");
  DEBUG.print(f,4);
  DEBUG.println(" to bytes for eeprom storage.");

  cvt eepromfloat;
  eepromfloat.val = f;

  EEPROM.write(address,eepromfloat.b[0]);
  EEPROM.write(address+1,eepromfloat.b[1]); 
  EEPROM.write(address+2,eepromfloat.b[2]); 
  EEPROM.write(address+3,eepromfloat.b[3]); 
  DEBUG.println(eepromfloat.b[0]);
  DEBUG.println(eepromfloat.b[1]);
  DEBUG.println(eepromfloat.b[2]);
  DEBUG.println(eepromfloat.b[3]);

}

float CalibrationClass::floatFromEeprom(int address) {
  //return NAN;  // Returning this will reset calibration data to default. 
    cvt eepromfloat;

  eepromfloat.b[0] = EEPROM.read(address);
  eepromfloat.b[1] = EEPROM.read(address+1);
  eepromfloat.b[2] = EEPROM.read(address+2);
  eepromfloat.b[3] = EEPROM.read(address+3);
  return eepromfloat.val;
}

void CalibrationClass::adjDacGainCompPos(float val) {
  dacGainCompPos += val;
  floatToEeprom(ea_dac_gain_comp_pos, dacGainCompPos);
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Dac gain pos comp at address ");
  DEBUG.print(ea_dac_gain_comp_pos,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(dacGainCompPos,6);
  DEBUG.flush();
}

void CalibrationClass::adjDacGainCompPos2(float val) {
  dacGainCompPos2 += val;
  floatToEeprom(ea_dac_gain_comp_pos2, dacGainCompPos2);
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Dac gain pos comp 2 at address ");
  DEBUG.print(ea_dac_gain_comp_pos2,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(dacGainCompPos2,6);
  DEBUG.flush();
}


float CalibrationClass::getDacGainCompPos() {
  return dacGainCompPos;
}

float CalibrationClass::getDacGainCompPos2() {
  return dacGainCompPos2;
}

void CalibrationClass::adjDacGainCompNeg(float val) {
  dacGainCompNeg += val;
  floatToEeprom(ea_dac_gain_comp_neg, dacGainCompNeg);
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
   DEBUG.print("Dac gain neg comp at address ");
  DEBUG.print(ea_dac_gain_comp_neg,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(dacGainCompNeg,6);
  DEBUG.flush();
}

void CalibrationClass::adjDacGainCompNeg2(float val) {
  dacGainCompNeg2 += val;
  floatToEeprom(ea_dac_gain_comp_neg2, dacGainCompNeg2);
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
   DEBUG.print("Dac gain neg 2 comp at address ");
  DEBUG.print(ea_dac_gain_comp_neg2,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(dacGainCompNeg2,6);
  DEBUG.flush();
}











float CalibrationClass::getDacGainCompNeg() {
  return dacGainCompNeg;
}

float CalibrationClass::getDacGainCompNeg2() {
  return dacGainCompNeg2;
}

void CalibrationClass::adjDacGainCompLim(float val) {
  dacGainCompLim += val;
  floatToEeprom(ea_dac_gain_comp_lim, dacGainCompLim);
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
   DEBUG.print("Dac gain lim comp at address ");
  DEBUG.print(ea_dac_gain_comp_lim,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(dacGainCompLim,6);
  DEBUG.flush();
}

float CalibrationClass::getDacGainCompLim() {
  return dacGainCompLim;
}



void CalibrationClass::adjAdcGainCompPos(float val) {
  adcGainCompPos += val;
  floatToEeprom(ea_adc_gain_comp_pos, adcGainCompPos);
  DEBUG.print("Operation type = ");
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Adc gain pos comp at address ");
  DEBUG.print(ea_adc_gain_comp_pos,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(adcGainCompPos,6);
  DEBUG.flush();
}

float CalibrationClass::getAdcGainCompPos() {
  return adcGainCompPos;
}

void CalibrationClass::adjAdcGainCompNeg(float val) {
  adcGainCompNeg += val;
  floatToEeprom(ea_adc_gain_comp_neg, adcGainCompNeg);
  DEBUG.print("Operation type = ");
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Adc gain neg comp at address ");
  DEBUG.print(ea_adc_gain_comp_neg,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(adcGainCompNeg,6);
  DEBUG.flush();
}

float CalibrationClass::getAdcGainCompNeg() {
  return adcGainCompNeg;
}




void CalibrationClass::adjAdcGainCompPos2(float val) {
  adcGainCompPos2 += val;
  floatToEeprom(ea_adc_gain_comp_pos2, adcGainCompPos2);
  DEBUG.print("Operation type = ");
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Adc gain pos comp at address ");
  DEBUG.print(ea_adc_gain_comp_pos2,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(adcGainCompPos2,6);
  DEBUG.flush();
}

float CalibrationClass::getAdcGainCompPos2() {
  return adcGainCompPos2;
}

void CalibrationClass::adjAdcGainCompNeg2(float val) {
  adcGainCompNeg2 += val;
  floatToEeprom(ea_adc_gain_comp_neg2, adcGainCompNeg2);
  DEBUG.print("Operation type = ");
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Adc gain neg comp at address ");
  DEBUG.print(ea_adc_gain_comp_neg2,HEX);
  DEBUG.print(" adjusted to:");
  DEBUG.println(adcGainCompNeg2,6);
  DEBUG.flush();
}

float CalibrationClass::getAdcGainCompNeg2() {
  return adcGainCompNeg2;
}



void CalibrationClass::adjDacZeroComp(float val) {
  dacZeroComp += val;
  floatToEeprom(ea_dac_zero_comp, dacZeroComp);
  DEBUG.print("Operation type = ");
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Dac zero comp at address ");
  DEBUG.print(ea_dac_zero_comp,HEX);
  DEBUG.print(" adjusted to:");  
  DEBUG.println(dacZeroComp,6);

  DEBUG.flush();
}

float CalibrationClass::getDacZeroComp() {
  return dacZeroComp;
}


void CalibrationClass::adjDacZeroComp2(float val) {
  dacZeroComp2 += val;
  floatToEeprom(ea_dac_zero_comp2, dacZeroComp2);
  DEBUG.print("Operation type = ");
  DEBUG.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  DEBUG.print("Dac zero comp2 at address ");
  DEBUG.print(ea_dac_zero_comp2, HEX);
  DEBUG.print(" adjusted to:");  
  DEBUG.println(dacZeroComp2,6);

  DEBUG.flush();
}

float CalibrationClass::getDacZeroComp2() {
  return dacZeroComp2;
}


void CalibrationClass::printNonlinearValues() {
       DEBUG.print("Nonlinear ADC data ");
       if (operationType == SOURCE_VOLTAGE) {
        DEBUG.println("Voltage");

       } else {
        DEBUG.println("Current");

       }

     DEBUG.print("adc_cal_points=");
    DEBUG.println(adc_cal_points);

   for (int i=0;i<adc_cal_points;i++) {
      float diff = set_adc[i] - meas_adc[i];
      DEBUG.print("Set ");
      DEBUG.print(set_adc[i]);
      DEBUG.print("mV, meas ");
      DEBUG.print(meas_adc[i]);
      DEBUG.print("mV, diff ");
      DEBUG.print(diff*1000.0);
      DEBUG.println("uV");
   }
}

void CalibrationClass::renderCal2(int x, int y, float valM, float setM, CURRENT_RANGE current_range, bool reduceDetails) {



 GD.ColorRGB(0xaaaaaa);
if (!reduceDetails) {
    // TODO: Differ between current ranges for voltage zero ?
    GD.cmd_text(x,y+45,27,0,"ADC Null V");
    DIGIT_UTIL.renderValue(x,  y+60 ,nullValueVol[0], 1, -1); 
    GD.cmd_text(x+150,y+45,27,0,"ADC NullV(10mA)");
    DIGIT_UTIL.renderValue(x + 150,  y+60 ,nullValueVol[1], 1, -1); 

    GD.cmd_text(x,y+35+60,27,0,"ADC Null C(1A)");
    DIGIT_UTIL.renderValue(x,  y+110 ,nullValueCur[0], 1, -1); 
    GD.cmd_text(x+150,y+35+60,27,0,"ADC Null C(10mA)");
    DIGIT_UTIL.renderValue(x + 150,  y+110 ,nullValueCur[1], 1, -1); 
    GD.Tag(0);

}
     GD.LineWidth(20);
  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0xff0000);


  float max_set_value = set_adc[adc_cal_points-1];
  float min_set_value = set_adc[0];

  float max_meas_value = meas_adc[adc_cal_points-1];

  //TODO Do real calculations to find these values based on number of points, width of graph etc....   
  float pixelsPrVolt = 145; // pixel width pr volt
  if (operationType == SOURCE_CURRENT) {
    pixelsPrVolt = 140;
  }
  float x_null_position = 450; // x position for 0V
  if (operationType == SOURCE_CURRENT) {
    x_null_position = 465;
  }
  float correction_display_factor = 80000.0; // TODO: Make it show as ppm ?  uV ?

  // avoid drawing graph outside of target area (can happen if cal values are large...)
  // Comment out the lines below if you need see "way-off" values while debugging/developing...
//  GD.ScissorXY(0,280);
//  GD.ScissorSize(640,150);


  y=y+10;
  // correction graph
  if (!reduceDetails) {
    for (int i=0;i<adc_cal_points;i++) {
        float diff = set_adc[i] - meas_adc[i];
      //  DEBUG.print("adc_cal_points=");
      //  DEBUG.print(adc_cal_points);
      //  DEBUG.print(", min_set_value=");
      //  DEBUG.print(min_set_value);
      //  DEBUG.print("max_set_value=");
      //  DEBUG.print(max_set_value);
      //  DEBUG.print(", Cal points ");
      //  DEBUG.print(adc_cal_points);
      //  DEBUG.print(" - Rendering set ");
      //  DEBUG.print(set_adc[i]);
      //  DEBUG.print(" meas ");
      //  DEBUG.println(meas_adc[i]);
        int xv = pixelsPrVolt *(set_adc[i] / max_set_value);
        int yv = /*150 *(meas_adc[i] / max_meas_value) - */(diff/max_meas_value) * correction_display_factor;
        GD.Vertex2ii(x+x_null_position+xv, y + 100 - yv);
    }
  }

  // voltage axis
  GD.ColorRGB(0x00ff00);
  GD.Begin(LINE_STRIP);
  GD.ColorA(100);
  if (reduceDetails){
//    int xv1 = pixelsPrVolt *(set_adc[0] / max_set_value);
//    int xv2 = pixelsPrVolt *(set_adc[adc_cal_points-1] / max_set_value);
//    GD.Vertex2ii(x+x_null_position+xv1,y+100);
//    GD.Vertex2ii(x+x_null_position+xv2,y+100);
  } else {
    for (int i=0;i<adc_cal_points;i++) {
        //float diff = set_adc[i] - meas_adc[i];
        int xv = pixelsPrVolt *(set_adc[i] / max_set_value);
        GD.Vertex2ii(x+x_null_position+xv, y + 100);
    }
  }
  
  // reset the scissor. Is this the way ????
  GD.ScissorXY(0,0);
  GD.ScissorSize(800,640);

  y=y-50;
  // voltage labels
  if (!reduceDetails) {
    if (operationType == SOURCE_VOLTAGE){
      for (int i=min_set_value/1000.0;i<=max_set_value/1000.0;i=i+4) {
          int xv = i* pixelsPrVolt/(int)(max_set_value/1000.0);
          GD.cmd_text(x+x_null_position+xv-10, y + 200, 27, 0, i<0?"-":" ");
          if (abs(i) >=10.0) {
            GD.cmd_number(x+x_null_position+xv, y + 200, 27, 2, abs(i));
          } else {
            GD.cmd_number(x+x_null_position+xv, y + 200, 27, 1, abs(i));
          }
      }
    } else {
       for (int i=min_set_value/100.0;i<=max_set_value/100.0;i=i+1) {
          int xv = i* pixelsPrVolt/(int)(max_set_value/100.0);
          GD.cmd_text(x+x_null_position+xv-10, y + 200, 27, 0, i<0?"-":" ");
         
          GD.cmd_number(x+x_null_position+xv, y + 200, 27, 3, abs(i*100));
         
      }
    }
  }



  x=x+350;
  y=y+50;
  GD.ColorRGB(0x000000);
    GD.ColorA(255);


    GD.Tag(BUTTON_DAC_NONLINEAR_CALIBRATE);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_NONLINEAR_CALIBRATE));
    GD.cmd_button(x+310,y+45,120,50,29,0,"AUTOCAL");
    GD.Tag(0);

    GD.Tag(BUTTON_DAC_ZERO_CALIBRATE);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_ZERO_CALIBRATE));
    GD.cmd_button(x+310,y+100,120,50,29,0,"ZEROCAL");
    
    GD.Tag(0);
    

}

void CalibrationClass::renderCal(int x, int y, float valM, float setM, CURRENT_RANGE current_range, bool reduceDetails)
{
 
  // dac vol2cur2
  if (current_range == MILLIAMP10 && operationType == SOURCE_CURRENT)
  {
    if (!reduceDetails)
    {
      GD.ColorRGB(0xaaaaaa);
      if (FILTERS->mean < 0)
      {
        GD.cmd_text(x + 10, y + 45, 27, 0, "DAC G10mA -");
        DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompNeg2() * 100.0, 1, -1);
        GD.cmd_text(x + 93, y + 68, 27, 0, "%");
      }
      else
      {
        GD.cmd_text(x + 10, y + 45, 27, 0, "DAC G10mA +");
        DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompPos2() * 100.0, 1, -1);
        GD.cmd_text(x + 93, y + 68, 27, 0, "%");
      }
    }
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_DAC_GAIN_COMP_UP2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_UP2));
    GD.cmd_button(x + 10, y + 95, 100, 50, 29, 0, "UP2");
    GD.Tag(BUTTON_DAC_GAIN_COMP_DOWN2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_DOWN2));
    GD.cmd_button(x + 10, y + 155, 100, 50, 29, 0, "DOWN2");
    GD.Tag(0);
  }

  // TODO: HACK!  Using MILLIAMP10 to indicate high voltage range !!!!  Stupid !   Should be fixed...
  else if (current_range == MILLIAMP10 && operationType == SOURCE_VOLTAGE)
  {
    if (!reduceDetails)
    {
      GD.ColorRGB(0xaaaaaa);
      if (FILTERS->mean < 0)
      {
        GD.cmd_text(x + 10, y + 45, 27, 0, "DAC 10V -");
        DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompNeg2() * 100.0, 1, -1);
        GD.cmd_text(x + 93, y + 68, 27, 0, "%");
      }
      else
      {
        GD.cmd_text(x + 10, y + 45, 27, 0, "DAC 10V +");
        DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompPos2() * 100.0, 1, -1);
        GD.cmd_text(x + 93, y + 68, 27, 0, "%");
      }
    }
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_DAC_GAIN_COMP_UP2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_UP2));
    GD.cmd_button(x + 10, y + 95, 100, 50, 29, 0, "UP2");
    GD.Tag(BUTTON_DAC_GAIN_COMP_DOWN2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_DOWN2));
    GD.cmd_button(x + 10, y + 155, 100, 50, 29, 0, "DOWN2");
    GD.Tag(0);
  }

  else
  {

    if (!reduceDetails)
    {
      GD.ColorRGB(0xaaaaaa);
      if (FILTERS->mean < 0)
      {
        GD.cmd_text(x + 10, y + 45, 27, 0, "DAC GAIN -");
        DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompNeg() * 100.0, 1, -1);
        GD.cmd_text(x + 93, y + 68, 27, 0, "%");
      }
      else
      {
        GD.cmd_text(x + 10, y + 45, 27, 0, "DAC GAIN +");
        DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompPos() * 100.0, 1, -1);
        GD.cmd_text(x + 93, y + 68, 27, 0, "%");
      }
    }
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_DAC_GAIN_COMP_UP);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_UP));
    GD.cmd_button(x + 10, y + 95, 100, 50, 29, 0, "UP");
    GD.Tag(BUTTON_DAC_GAIN_COMP_DOWN);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_DOWN));
    GD.cmd_button(x + 10, y + 155, 100, 50, 29, 0, "DOWN");
    GD.Tag(0);
  }

  x = x + 10;

  if (current_range == MILLIAMP10 && operationType == SOURCE_CURRENT)
  {

    if (!reduceDetails)
    {

      GD.ColorRGB(0xaaaaaa);

      if (FILTERS->mean < 0)
      {
        GD.cmd_text(x + 120, y + 45, 27, 0, "ADC G10M -");
        DIGIT_UTIL.renderValue(x + 110, y + 65, getAdcGainCompNeg2() * 5.0, 1, -1);
        GD.cmd_text(x + 203, y + 68, 27, 0, "%");
      }
      else
      {
        GD.cmd_text(x + 120, y + 45, 27, 0, "ADC G10M +");
        DIGIT_UTIL.renderValue(x + 110, y + 65, getAdcGainCompPos2() * 5.0, 1, -1);
        GD.cmd_text(x + 203, y + 68, 27, 0, "%");
      }
    }
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_ADC_GAIN_COMP_UP2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_ADC_GAIN_COMP_UP2));
    GD.cmd_button(x + 120, y + 95, 100, 50, 29, 0, "UP");
    GD.Tag(BUTTON_ADC_GAIN_COMP_DOWN2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_ADC_GAIN_COMP_DOWN2));
    GD.cmd_button(x + 120, y + 155, 100, 50, 29, 0, "DOWN");
    GD.Tag(0);
  }

  // TODO: HACK!  Using MILLIAMP10 to indicate high voltage range !!!!  Stupid !   Should be fixed...
  // WARNING: Not relevant because we dont use multiple measurement ranges.
  //          However, it can be used as a poor mans nonlinear gain
  //          compensation over and above a certain value....
  // TODO: Remove this special handling ?
  else if (current_range == MILLIAMP10 && operationType == SOURCE_VOLTAGE)
  {
    if (!reduceDetails)
    {

      GD.ColorRGB(0xaaaaaa);

      if (FILTERS->mean < 0)
      {
        GD.cmd_text(x + 120, y + 45, 27, 0, "ADC GAIN2 -");
        DIGIT_UTIL.renderValue(x + 110, y + 65, getAdcGainCompNeg2() * 100.0, 1, -1);
        GD.cmd_text(x + 203, y + 68, 27, 0, "%");
      }
      else
      {
        GD.cmd_text(x + 120, y + 45, 27, 0, "ADC GAIN2 +");
        DIGIT_UTIL.renderValue(x + 110, y + 65, getAdcGainCompPos2() * 100.0, 1, -1);
        GD.cmd_text(x + 203, y + 68, 27, 0, "%");
      }
    }
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_ADC_GAIN_COMP_UP2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_ADC_GAIN_COMP_UP2));
    GD.cmd_button(x + 120, y + 95, 100, 50, 29, 0, "UPv2");
    GD.Tag(BUTTON_ADC_GAIN_COMP_DOWN2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_ADC_GAIN_COMP_DOWN2));
    GD.cmd_button(x + 120, y + 155, 100, 50, 29, 0, "DOWNv2");
    GD.Tag(0);
  }
  else 
  {
    if (!reduceDetails)
    {

      GD.ColorRGB(0xaaaaaa);

      if (FILTERS->mean < 0)
      {
        GD.cmd_text(x + 120, y + 45, 27, 0, "ADC GAIN -");
        DIGIT_UTIL.renderValue(x + 110, y + 65, getAdcGainCompNeg() * 100.0, 1, -1);
        GD.cmd_text(x + 203, y + 68, 27, 0, "%");
      }
      else
      {
        GD.cmd_text(x + 120, y + 45, 27, 0, "ADC GAIN +");
        DIGIT_UTIL.renderValue(x + 110, y + 65, getAdcGainCompPos() * 100.0, 1, -1);
        GD.cmd_text(x + 203, y + 68, 27, 0, "%");
      }
    }
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_ADC_GAIN_COMP_UP);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_ADC_GAIN_COMP_UP));
    GD.cmd_button(x + 120, y + 95, 100, 50, 29, 0, "UP");
    GD.Tag(BUTTON_ADC_GAIN_COMP_DOWN);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_ADC_GAIN_COMP_DOWN));
    GD.cmd_button(x + 120, y + 155, 100, 50, 29, 0, "DOWN");
    GD.Tag(0);
  }

  x = x + 120;
  if (!reduceDetails)
  {

    GD.ColorRGB(0xaaaaaa);

    if (current_range == MILLIAMP10 && operationType == SOURCE_CURRENT)
    {
      GD.cmd_text(x + 120, y + 45, 27, 0, "DAC z10m");
      DIGIT_UTIL.renderValue(x + 110, y + 65, getDacZeroComp2(), 1, -1);
      GD.cmd_text(x + 203, y + 68, 27, 0, "");
    }
    else
    {
      GD.cmd_text(x + 120, y + 45, 27, 0, "DAC zero");
      DIGIT_UTIL.renderValue(x + 110, y + 65, getDacZeroComp(), 1, -1);
      GD.cmd_text(x + 203, y + 68, 27, 0, "");
    }
  }
  GD.ColorRGB(0x000000);

  if (current_range == MILLIAMP10 && operationType == SOURCE_CURRENT)
  {

    GD.Tag(BUTTON_DAC_ZERO_COMP_UP2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_ZERO_COMP_UP2));
    GD.cmd_button(x + 120, y + 95, 100, 50, 29, 0, "UP2");
    GD.Tag(BUTTON_DAC_ZERO_COMP_DOWN2);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_ZERO_COMP_DOWN2));
    GD.cmd_button(x + 120, y + 155, 100, 50, 29, 0, "DOWN2");
  }
  else
  {
    GD.Tag(BUTTON_DAC_ZERO_COMP_UP);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_ZERO_COMP_UP));
    GD.cmd_button(x + 120, y + 95, 100, 50, 29, 0, "UP");
    GD.Tag(BUTTON_DAC_ZERO_COMP_DOWN);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_ZERO_COMP_DOWN));
    GD.cmd_button(x + 120, y + 155, 100, 50, 29, 0, "DOWN");
  }

  GD.Tag(0); // Seems to fix problem with function calles when touching a special place on the screen... why ????

  x = x + 230;
  if (!reduceDetails)
  {

    GD.ColorRGB(0xaaaaaa);
    GD.cmd_text(x + 10, y + 45, 27, 0, "LIM GAIN");
    DIGIT_UTIL.renderValue(x + 0, y + 65, getDacGainCompLim() * 100.0, 1, -1);
    GD.cmd_text(x + 93, y + 68, 27, 0, "%");
    GD.ColorRGB(0x000000);
  }
  GD.Tag(BUTTON_DAC_GAIN_COMP_LIM_UP);
  GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_LIM_UP));
  GD.cmd_button(x + 10, y + 95, 100, 50, 29, 0, "UP");
  GD.Tag(BUTTON_DAC_GAIN_COMP_LIM_DOWN);
  GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_DAC_GAIN_COMP_LIM_DOWN));
  GD.cmd_button(x + 10, y + 155, 100, 50, 29, 0, "DOWN");
  GD.Tag(0);
}


bool CalibrationClass::handleCalibrationButtons(int tag, OPERATION_TYPE operationType, CURRENT_RANGE currentRange) {

    int repeatDelay = 50;
    int valueToReturnIfTooFast = false;

    if (tag == BUTTON_DAC_GAIN_COMP_UP) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjDacGainCompNeg(0.000005);
         } else {
            V_CALIBRATION.adjDacGainCompPos(0.000005);
         }
         GD.__end();
         //if (SMU1->fltSetCommitVoltageSource(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitVoltageSource(mv*1000, true);

         GD.resume();
       } else {
          if (mv < 0) {
            C_CALIBRATION.adjDacGainCompNeg(0.00001);
         } else {
            C_CALIBRATION.adjDacGainCompPos(0.00001);
         }
         GD.__end();
         //if (SMU1->fltSetCommitCurrentSource(mv*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentSource(mv*1000);

         GD.resume();
       }
    } 
    
    else if (tag == BUTTON_DAC_GAIN_COMP_UP2) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjDacGainCompNeg2(0.000005);
         } else {
            V_CALIBRATION.adjDacGainCompPos2(0.000005);
         }
         GD.__end();
         //if (SMU1->fltSetCommitVoltageSource(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitVoltageSource(mv*1000, true);

         GD.resume();
       } else {
          if (mv < 0) {
            C_CALIBRATION.adjDacGainCompNeg2(0.000005);
         } else {
            C_CALIBRATION.adjDacGainCompPos2(0.000005);
         }
         GD.__end();
         //if (SMU1->fltSetCommitCurrentSource(mv*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentSource(mv*1000);

         GD.resume();
       }
    } 

    else if (tag == BUTTON_DAC_GAIN_COMP_DOWN) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
      } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjDacGainCompNeg(-0.000005);
         } else {
            V_CALIBRATION.adjDacGainCompPos(-0.000005);
         }
         //if (operationType == SOURCE_VOLTAGE) {
         GD.__end();
         //if (SMU1->fltSetCommitVoltageSource(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitVoltageSource(mv*1000, true);

         GD.resume();
       } else {
          if (mv < 0) {
            C_CALIBRATION.adjDacGainCompNeg(-0.00001 );
         } else {
            C_CALIBRATION.adjDacGainCompPos(-0.00001 );
         }
         GD.__end();
         //if (SMU1->fltSetCommitCurrentSource(mv*1000.0)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentSource(mv*1000.0);
         GD.resume();
       }   
    } 
    
    else if (tag == BUTTON_DAC_GAIN_COMP_DOWN2) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
      } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         if (mv < 0) {
            V_CALIBRATION.adjDacGainCompNeg2(-0.000005);
         } else {
            V_CALIBRATION.adjDacGainCompPos2(-0.000005);
         }
         //if (operationType == SOURCE_VOLTAGE) {
         GD.__end();
         //if (SMU1->fltSetCommitVoltageSource(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitVoltageSource(mv*1000, true);
         GD.resume();
       } else {
          if (mv < 0) {
            C_CALIBRATION.adjDacGainCompNeg2(-0.000005);
         } else {
            C_CALIBRATION.adjDacGainCompPos2(-0.000005);
         }
         GD.__end();
         //if (SMU1->fltSetCommitCurrentSource(mv*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentSource(mv*1000);
         GD.resume();
       }   
    } 
 
    else if (tag == BUTTON_DAC_GAIN_COMP_LIM_UP) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
      } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getLimitValue_micro();
       if (operationType == SOURCE_VOLTAGE) {
         V_CALIBRATION.adjDacGainCompLim(+0.000005*10.0);
         
         //if (operationType == SOURCE_VOLTAGE) {
         GD.__end();
         //if (SMU1->fltSetCommitCurrentLimit(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentLimit(mv);
         GD.resume();
       }
    } 
    
    else if (tag == BUTTON_DAC_GAIN_COMP_LIM_DOWN) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
      } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getLimitValue_micro();
       if (operationType == SOURCE_VOLTAGE) {
         V_CALIBRATION.adjDacGainCompLim(-0.000005*10.0);
         
         //if (operationType == SOURCE_VOLTAGE) {
         GD.__end();
         //if (SMU1->fltSetCommitCurrentLimit(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentLimit(mv);
         GD.resume();
       }   
    } 
    
    else if (tag == BUTTON_ADC_GAIN_COMP_UP) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
        
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         mv = V_STATS.rawValue;
         if (mv < 0) {
            V_CALIBRATION.adjAdcGainCompNeg(0.000002);
         } else {
            V_CALIBRATION.adjAdcGainCompPos(0.000002);
         }
       } else {
         mv = C_STATS.rawValue;
         if (mv < 0) {
            C_CALIBRATION.adjAdcGainCompNeg(0.000005 *2.0);
         } else {
            C_CALIBRATION.adjAdcGainCompPos(0.000005 *2.0);
         }
       }
      
    } 
    else if (tag == BUTTON_ADC_GAIN_COMP_DOWN) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         mv = V_STATS.rawValue;
         if (mv < 0) {
            V_CALIBRATION.adjAdcGainCompNeg(-0.000001 * 10.0);
         } else {
            V_CALIBRATION.adjAdcGainCompPos(-0.000001 * 10.0);
         }
       } else {
        mv = C_STATS.rawValue;
        if (mv < 0) {
            C_CALIBRATION.adjAdcGainCompNeg(-0.000005 *2.0);
         } else {
            C_CALIBRATION.adjAdcGainCompPos(-0.000005 *2.0);
         }
       }
      
    } 

    else if (tag == BUTTON_ADC_GAIN_COMP_UP2) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
        
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         mv = V_STATS.rawValue;
         if (mv < 0) {
            V_CALIBRATION.adjAdcGainCompNeg2(0.000001);
         } else {
            V_CALIBRATION.adjAdcGainCompPos2(0.000001);
         }
       } else {
         mv = C_STATS.rawValue;
         if (mv < 0) {
            C_CALIBRATION.adjAdcGainCompNeg2(currentRange==MILLIAMP10 ? 0.000005 : 0.000002);
         } else {
            C_CALIBRATION.adjAdcGainCompPos2(currentRange==MILLIAMP10 ? 0.000005 : 0.000002);
         }
       }
      
    } 
    else if (tag == BUTTON_ADC_GAIN_COMP_DOWN2) {
       if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         mv = V_STATS.rawValue;
         if (mv < 0) {
            V_CALIBRATION.adjAdcGainCompNeg2(-0.000001);
         } else {
            V_CALIBRATION.adjAdcGainCompPos2(-0.000001);
         }
       } else {
        mv = C_STATS.rawValue;
        if (mv < 0) {
            C_CALIBRATION.adjAdcGainCompNeg2(currentRange==MILLIAMP10 ? -0.000005 : -0.000002);
         } else {
            C_CALIBRATION.adjAdcGainCompPos2(currentRange==MILLIAMP10 ? -0.000005 : -0.000001);
         }
       }
      
    } 
    
    
    
    else if (tag == BUTTON_DAC_ZERO_COMP_UP or tag == BUTTON_DAC_ZERO_COMP_UP2 ) {
      if (timeSinceLastChange + repeatDelay > millis()){
        return valueToReturnIfTooFast;
      } 
      timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
          V_CALIBRATION.adjDacZeroComp(+0.000002);
          GD.__end();
         //if (SMU1->fltSetCommitVoltageSource(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitVoltageSource(mv*1000, true);
         GD.resume();
         
       } else {
          if (tag == BUTTON_DAC_ZERO_COMP_UP) {
            C_CALIBRATION.adjDacZeroComp(+0.000001);
          } else {
            C_CALIBRATION.adjDacZeroComp2(+0.000002);
          }
          GD.__end();
         //if (SMU1->fltSetCommitCurrentSource(mv*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentSource(mv*1000);

         GD.resume();    
       }
    }
    else if (tag == BUTTON_DAC_ZERO_COMP_DOWN or tag == BUTTON_DAC_ZERO_COMP_DOWN2) {
       if (timeSinceLastChange + repeatDelay > millis()){
         return valueToReturnIfTooFast;
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);
       float mv = SMU1->getSetValue_micro()/1000.0;
       if (operationType == SOURCE_VOLTAGE) {
         V_CALIBRATION.adjDacZeroComp(-0.000002);
         GD.__end();
         //if (SMU1->fltSetCommitVoltageSource(mv*1000, true)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitVoltageSource(mv*1000, true);
         GD.resume();
       } else {
         if (tag == BUTTON_DAC_ZERO_COMP_DOWN) {
           C_CALIBRATION.adjDacZeroComp(-0.000001);
         } else {
           C_CALIBRATION.adjDacZeroComp2(-0.000002);
         }
         GD.__end();
         //if (SMU1->fltSetCommitCurrentSource(mv*1000)) printError(_PRINT_ERROR_VOLTAGE_SOURCE_SETTING);
         SMU1->fltSetCommitCurrentSource(mv*1000);
         GD.resume(); 
       }
    }
    else if (tag == BUTTON_DAC_NONLINEAR_CALIBRATE) {
       if (timeSinceLastChange + 1000 > millis()){
        return valueToReturnIfTooFast;
       } 
       timeSinceLastChange = millis();
       DIGIT_UTIL.startIndicator(tag);

       if (operationType == SOURCE_VOLTAGE) {
         DEBUG.println("Start auto calibration of dac non linearity in voltage source mode");
         V_CALIBRATION.startAutoCal();
       } else {
         DEBUG.println("Start auto calibration of dac non linearity in current source mode");
         C_CALIBRATION.startAutoCal();
       }
      
    } 
    return true; // normal
}
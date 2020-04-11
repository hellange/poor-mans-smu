#include "Calibration.h"
#include "Arduino.h"
#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include <EEPROM.h>
#include "eeprom_adr.h"
#include "Filters.h"
#include "digit_util.h"
#include "SMU_HAL_717x.h"

CalibrationClass V_CALIBRATION;
CalibrationClass C_CALIBRATION;

extern ADCClass SMU[];

  
  void float_array_init(float *a, const int ct, ...) {
  va_list args;
  va_start(args, ct);
  for(int i = 0; i < ct; ++i) {
    double  d = (float)va_arg(args,double);
    a[i] = d;
    Serial.print(d,2);
    Serial.print("(");
    Serial.print(a[i],2);
    Serial.print(")");

    if (i == ct-1) {
      Serial.print(",");
    }
  }
  Serial.println("");
  va_end(args);
}

// using static to share among instances. TODO: Fix this mess
float CalibrationClass::nullValueVol[2];
float CalibrationClass::nullValueCur[2];

void CalibrationClass::init(OPERATION_TYPE operationType_) {
  operationType = operationType_;

  ea_dac_zero_comp = EA_DAC_ZERO_COMP_VOL;

  // TODO: Differ between constant current and constant voltage nulling
  ea_adc_zero_comp_vol = EA_ADC_ZERO_COMP_VOL;
  ea_adc_zero_comp_cur = EA_ADC_ZERO_COMP_CUR;
  ea_adc_zero_comp_cur2 = EA_ADC_ZERO_COMP_CUR2;

  
  if (operationType == SOURCE_VOLTAGE) {
    // set value
//    adc_cal_points = 37;
//    Serial.println("Voltage measurement nonlinearity:");
//    float_array_init(set_adc, adc_cal_points,  -12000.00, -11000.00, -10000.00, -9000.00, -8000.00, -7000.00, -6000.00, -5000.00, -4000.00, -3500.00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, -50.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 2000.00, 2500.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00, 12000.00);
//    float_array_init(meas_adc, adc_cal_points, -11992.94, -10996.74, -10000.00, -8999.41, -7998.84, -6998.41, -5996.70, -4995.87, -3996.70, -3497.11, -2997.69, -2497.78, -1997.91, -0999.54, -499.96, -100.01, -50.00, 0.00, 50.06, 100.16, 250.34, 400.54, 499.94, 899.69, 0999.60, 1997.99, 2497.91, 2997.80, 3997.05, 4996.08, 5996.35, 6999.66, 8000.00, 8994.87,  9985.00, 10977.66, 11986.00);
//
//    dac_cal_points = 26;
//    Serial.println("Voltage source nonlinearty:");
//    float_array_init(set_dac, dac_cal_points,  -11000.00, -10000.00, -5000.00, -4000.00, -3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00);
//    float_array_init(meas_dac, dac_cal_points, -11000.00, -10000.00, -5000.00, -3999.98, -2999.90, -2000.14, -1000.07, 0.00, 5.00, 49.99, 100.02, 500.00, 0999.98, 1999.97, 3000.00, 3500.20, 4000.08, 4200.06, 4500.05, 5000.04, 6000.01, 7000.17, 8000.12, 9000.10, 10000.00, 11000.00);
  
    ea_dac_gain_comp_pos = EA_DAC_GAIN_COMP_POS_VOL;
    ea_dac_gain_comp_neg = EA_DAC_GAIN_COMP_NEG_VOL;
    ea_adc_gain_comp_pos = EA_ADC_GAIN_COMP_POS_VOL;
    ea_adc_gain_comp_neg = EA_ADC_GAIN_COMP_NEG_VOL;

  

    ea_adc_nonlinear_comp_nr = EA_ADC_NONLINEAR_COMP_NR_VOL;
    ea_adc_nonlinear_comp_start = EA_ADC_NONLINEAR_COMP_START_VOL;

    FILTERS = &V_FILTERS;

    readAdcCalFromEeprom();

    
  } else {

        // set value
//    adc_cal_points = 15;
//    Serial.println("Current measurement nonlinearity:");
//    float_array_init(set_adc, adc_cal_points, -500.00, -400.00, -300.00, -200.00, -100.00, -50.00, -10.00, 0.00, 10.00, 50.00, 100.00, 200.00, 300.00, 400.00, 500.00 );
//    float_array_init(meas_adc, adc_cal_points,-500.00, -400.17, -300.09, -199.93, -099.96, -50.00, -10.00, 0.00, 10.02, 50.10, 100.18, 200.36, 300.69, 400.88, 500.00 );
//
//    dac_cal_points = 15;
//    Serial.println("Current source nonlinearity:");
//    float_array_init(set_dac, dac_cal_points,   -500.00, -400.00, -300.00, -200.00, -100.00, -50.00, -10.000, 0.00, 10.00, 50.00, 100.00, 200.00, 300.00, 400.00, 500.00);
//    float_array_init(meas_dac, dac_cal_points,  -500.00, -399.94, -299.90, -199.93, -099.94, -49.92, -10.006, 0.00, 10.00, 49.99, 099.96, 199.93, 299.97, 399.94, 500.00);

    ea_dac_gain_comp_pos = EA_DAC_GAIN_COMP_POS_CUR;
    ea_dac_gain_comp_neg = EA_DAC_GAIN_COMP_NEG_CUR;
    ea_adc_gain_comp_pos = EA_ADC_GAIN_COMP_POS_CUR;
    ea_adc_gain_comp_neg = EA_ADC_GAIN_COMP_NEG_CUR;

   

    ea_adc_nonlinear_comp_nr = EA_ADC_NONLINEAR_COMP_NR_CUR;
    ea_adc_nonlinear_comp_start = EA_ADC_NONLINEAR_COMP_START_CUR;

    FILTERS = &C_FILTERS;

    readAdcCalFromEeprom();

  }

  nullValueVol[0] = 0.0;
  nullValueVol[1] = 0.0;
  nullValueCur[0] = 0.0;
  nullValueCur[1] = 0.0;
  timeSinceLastChange = millis();
  
  dacGainCompPos = floatFromEeprom(ea_dac_gain_comp_pos);
 Serial.print("Read dacGainCompPos from eeprom address ");
  Serial.print(ea_dac_gain_comp_pos,HEX);
  Serial.print(":");
  if (isnan(dacGainCompPos)) {
    Serial.print("Not defined. Write default value:");
    dacGainCompPos = 1.0;
    floatToEeprom(ea_dac_gain_comp_pos,dacGainCompPos); // write initial default
  }
  Serial.println(dacGainCompPos,7);
 
  dacGainCompNeg = floatFromEeprom(ea_dac_gain_comp_neg);
  Serial.print("Read dacGainCompNeg from eeprom address ");
  Serial.print(ea_dac_gain_comp_neg,HEX);
  Serial.print(":");
  if (isnan(dacGainCompNeg)) {
    Serial.print("Not defined. Write default value:");
    dacGainCompNeg = 1.0;
    floatToEeprom(ea_dac_gain_comp_neg,dacGainCompNeg); // write initial default
  }
  Serial.println(dacGainCompNeg,7);


 adcGainCompPos = floatFromEeprom(ea_adc_gain_comp_pos);
 Serial.print("Read adcGainCompPos from eeprom address ");
  Serial.print(ea_adc_gain_comp_pos,HEX);
  Serial.print(":");  
  if (isnan(adcGainCompPos)) {
    Serial.print("Not defined. Write default value:");
    adcGainCompPos = 1.0;
    floatToEeprom(ea_adc_gain_comp_pos,adcGainCompPos); // write initial default
  }
  Serial.println(adcGainCompPos,7);
 
  adcGainCompNeg = floatFromEeprom(ea_adc_gain_comp_neg);
 Serial.print("Read adcGainCompNeg from eeprom address ");
  Serial.print(ea_adc_gain_comp_neg,HEX);
  Serial.print(":");  
  if (isnan(adcGainCompNeg)) {
    Serial.print("Not defined. Write default value:");
    adcGainCompNeg = 1.0;
    floatToEeprom(ea_adc_gain_comp_neg,adcGainCompNeg); // write initial default
  }
  Serial.println(adcGainCompNeg,7);

 dacZeroComp = floatFromEeprom(ea_dac_zero_comp);
 Serial.print("Read dacZeroComp from eeprom address ");
  Serial.print(ea_dac_zero_comp,HEX);
  Serial.print(":");
  if (isnan(dacZeroComp)) {
    Serial.print("Not defined. Write default value:");
    dacZeroComp = 0.0; // use millivolt
    floatToEeprom(ea_dac_zero_comp,dacZeroComp); // write initial default
  } else if (abs(dacZeroComp) < -10000.0 or abs(dacZeroComp) > 10000.0) {
    Serial.print("WARNING: Suspect dac zero value:");
    Serial.println(dacZeroComp);
    dacZeroComp = 0.0;
    Serial.print("Setting dac zero to:");
    Serial.println(dacZeroComp);
    
  }
  Serial.println(dacZeroComp,7);





 float adcZeroCompVol = floatFromEeprom(ea_adc_zero_comp_vol);
  
 Serial.print("Read adcZeroCompVol from eeprom address ");
  Serial.print(ea_adc_zero_comp_vol,HEX);
  Serial.print(":");
  if (isnan(adcZeroCompVol)) {
    Serial.print("Not defined. Write default value:");
    adcZeroCompVol = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_vol,adcZeroCompVol); // write initial default
  } else if (abs(adcZeroCompVol) < -10000.0 or abs(adcZeroCompVol) > 10000.0) {
    Serial.print("WARNING: Suspect adc zero value:");
    Serial.println(adcZeroCompVol);
    adcZeroCompVol = 0.0;
    Serial.print("Setting adc zero to:");
    Serial.println(adcZeroCompVol);
    
  }
  Serial.println(adcZeroCompVol,7);
  //TODO: Differ between the two null value (ranges)
  nullValueVol[0] = adcZeroCompVol;
  nullValueVol[1] = adcZeroCompVol;





   float adcZeroCompCur = floatFromEeprom(ea_adc_zero_comp_cur);
   float adcZeroCompCur2 = floatFromEeprom(ea_adc_zero_comp_cur2);
  
  Serial.print("Read adcZeroCompCur from eeprom address ");
  Serial.print(ea_adc_zero_comp_cur,HEX);
  Serial.print(":");
  if (isnan(adcZeroCompCur)) {
    Serial.print("Not defined. Write default value:");
    adcZeroCompCur = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_cur, adcZeroCompCur); // write initial default
  } else if (abs(adcZeroCompCur) < -10000.0 or abs(adcZeroCompCur) > 10000.0) {
    Serial.print("WARNING: Suspect adc zero value:");
    Serial.println(adcZeroCompCur);
    adcZeroCompCur = 0.0;
    Serial.print("Setting adc zero to:");
    Serial.println(adcZeroCompCur);
  }
  Serial.println(adcZeroCompCur,7);
  //TODO: Differ between the two null value (ranges)
  nullValueCur[0] = adcZeroCompCur;




 Serial.print("Read adcZeroCompCur2 from eeprom address ");
  Serial.print(ea_adc_zero_comp_cur2,HEX);
  Serial.print(":");
  if (isnan(adcZeroCompCur2)) {
    Serial.print("Not defined. Write default value:");
    adcZeroCompCur2 = 0.0; // use millivolt
    floatToEeprom(ea_adc_zero_comp_cur2, adcZeroCompCur2); // write initial default
  } else if (abs(adcZeroCompCur2) < -10000.0 or abs(adcZeroCompCur2) > 10000.0) {
    Serial.print("WARNING: Suspect adc zero value:");
    Serial.println(adcZeroCompCur2);
    adcZeroCompCur = 0.0;
    Serial.print("Setting adc zero to:");
    Serial.println(adcZeroCompCur2);
  }
  Serial.println(adcZeroCompCur2,7);
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
 float maxValueCal = 16000.00;
 float minValueCal = -16000.00;
 float stepValueCal = 1000.00;
 
void CalibrationClass::startAutoCal() {
  // use different values for current
  if (operationType == SOURCE_CURRENT) {
    maxValueCal = 400.0;
    minValueCal = -400.0;
    stepValueCal = 25.0;
  }
  
  if (!autoCalInProgress) {
    Serial.print("STARTING AUTO CALIBRATION OF ADC FOR ");
    Serial.println(operationType == SOURCE_CURRENT ? "CURRENT SOURCE" : "VOLTAGE SOURCE");
    autoCalInProgress = true;
    autoCalDacTimer = millis();
    autoCalV = minValueCal;
    useCalibratedValues = false;
    autoCalArrayPointer = 0;
    SMU[0].setSamplingRate(20);
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
  Serial.println(" READING NONLINEAR CALIBRATION ENTRIES FROM EEPROM");
  Serial.println(" ==================================================");
  int dataPtr = 0;
  float nr = floatFromEeprom(ea_adc_nonlinear_comp_nr);
  if (isnan(nr)) {
    Serial.println("WARNING: No calibration data found in eeprom. Use default values...");
    return;
  }
  if (nr < 10.0 or nr > 1000.0) {
    Serial.print("WARNING: Suspect data. Expecting a reasonable number of calibration entries. Not ");
    Serial.println((int)nr);
    return;
  }
 
  int nrOfPoints = (int)nr;
  Serial.print("Expecting ");
  Serial.print(nrOfPoints);
  Serial.println(" calibration entries in eeprom!");
  adc_cal_points = nrOfPoints;
  
  for (int adr = ea_adc_nonlinear_comp_start; adr < ea_adc_nonlinear_comp_start + nrOfPoints*8; adr = adr + 8) {
    float set = floatFromEeprom(adr);
    float meas = floatFromEeprom(adr+4);
    if (isnan(set) or isnan(meas)) {
      Serial.println("ERROR: No ADC nonlinear calibration data found!");
      Serial.print("       Current set value:");
      Serial.println(set);
      break;
    }
    set_adc[dataPtr] = set;
    meas_adc[dataPtr] = meas;
    Serial.print("Address:");
    Serial.print(adr);
    Serial.print(" READ set:");
    Serial.print(set,3);
    Serial.print(", meas:");
    Serial.println(meas,3);
    dataPtr ++;
  }
  Serial.print("Read ");
  Serial.print(nrOfPoints);
  Serial.print(" calibration entries from address ");
  Serial.print(ea_adc_nonlinear_comp_start);
  Serial.print(" to ");
  Serial.println(ea_adc_nonlinear_comp_start + nrOfPoints*8);
  useCalibratedValues = true;
  }

void CalibrationClass::writeAdcCalToEeprom(int nrOfPoints) {
  int dataPtr = 0;
  floatToEeprom(ea_adc_nonlinear_comp_nr, (float)nrOfPoints);
  for (int adr = ea_adc_nonlinear_comp_start; adr <= ea_adc_nonlinear_comp_start + nrOfPoints*8; adr = adr + 8) {
    floatToEeprom(adr, set_adc[dataPtr]);
    floatToEeprom(adr+4, meas_adc[dataPtr]);
    Serial.print("Address:");
    Serial.print(adr);
    Serial.print(" WRITE set:");
    Serial.print(set_adc[dataPtr],3);
    Serial.print(", meas:");
    Serial.println(meas_adc[dataPtr],3);
    dataPtr ++;
  }
  Serial.print("Wrote ");
  Serial.print(nrOfPoints);
  Serial.print(" calibration entries from address ");
  Serial.print(ea_adc_nonlinear_comp_start);
  Serial.print(" to ");
  Serial.println(ea_adc_nonlinear_comp_start + adc_cal_points*8);
  
}


void CalibrationClass::autoCalADCfromDAC() {
  if (autoCalInProgress == false) {
    return;
  } 


  if (!autoCalDone1 && autoCalDacTimer + 500 < millis()) {
    Serial.print("Set value to:");
    Serial.println(autoCalV);
    GD.__end();
    if (operationType == SOURCE_VOLTAGE) {
       SMU[0].fltSetCommitVoltageSource(autoCalV, true);
    } else {
       SMU[0].fltSetCommitCurrentSource(autoCalV);
    }
    GD.resume();

    set_adc[autoCalArrayPointer] = autoCalV;
    Serial.print("Set ");
    Serial.print(autoCalV);
    autoCalDone1 = true;
  }
  if (autoCalDacTimer + 5000 < millis()) {

    float v = FILTERS->mean;
  
    Serial.print(" > ");
    Serial.print(v);
    Serial.print(" (filter id:");
    Serial.print(FILTERS->id);
    Serial.println(")");


    Serial.print("Changed cal:");
    Serial.print(meas_adc[autoCalArrayPointer]);
    meas_adc[autoCalArrayPointer] = v;
    Serial.print(" to ");
    Serial.println(meas_adc[autoCalArrayPointer]);

    
     autoCalV += stepValueCal;
     autoCalArrayPointer ++;
    if (autoCalV > maxValueCal) {
      autoCalInProgress = false;
      autoCalDone = true;
      useCalibratedValues = true;
      adc_cal_points = autoCalArrayPointer;
      FILTERS->setFilterSize(5);
      Serial.print("DONE nonlinear calibration of ");
      Serial.print(adc_cal_points);
      Serial.println(" points");
      //writeAdcCalToEeprom(33); // TODO: autoCalArrayPointer ?
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
  floatToEeprom(ea_adc_zero_comp_vol, v);
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
  Serial.print("setRelativeValue to:");
  Serial.println(v);
  timeSinceLastChange = millis();

}
// todo: change parameter name to mv ?
float CalibrationClass::adc_nonlinear_compensation(float v){

  if (!useCalibratedValues) {
    //Serial.println("Dont use adc calibrated values");
    return v;
  } else {
    //Serial.println("Use calibrated values");
  }
  
  // Nonlinearity
  for (int i=0;i<adc_cal_points -1; i++) {
   // if (v > meas_adc[i] && v <= meas_adc[i+1]) {
    if (v > set_adc[i] && v <= set_adc[i+1]) {
//      Serial.print("Found within range ");
//      Serial.print(set_adc[i]);
//      Serial.print(":");
//      Serial.println(set_adc[i+1]);
      float adj_factor_low = set_adc[i] - meas_adc[i];
      float adj_factor_high = set_adc[i+1] - meas_adc[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_adc[i+1] - set_adc[i];
      float rangeStart = set_adc[i];
      float rangeEnd = set_adc[i+1];
      float partWithinRange = ( (v-set_adc[i]) / range); // 0 to 1. Where then 0.5 is in the middle of the range 
      if (partWithinRange < 0.0) {
        Serial.println("ERROR: calibration partWithinRange gave negative value...");
      }
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;
//      Serial.print("meas:");  
//      Serial.print(v, 3);
//      Serial.print(", range:");  
//      Serial.print(range, 3);
//      Serial.print(", part:");  
//      Serial.print(partWithinRange, 3);
//      Serial.print(", factor:");  
//      Serial.println(adj_factor, 3);
//
//      Serial.flush();
       
      v= v + adj_factor;
      return v;
    }
  }
  return v;
}



float CalibrationClass::dac_nonlinear_compensation(float milliVolt) {
  // Nonlinearity
  //Serial.print("Looking up in DAC comp table for ");
  //Serial.print(milliVolt);
  //Serial.println(" millivolt");
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

//      Serial.print("meas:");  
//      Serial.print(v, 4);
//      Serial.print(", range:");  
//      Serial.print(range, 4);
//      Serial.print(", part:");  
//      Serial.print(partWithinRange, 4);
//      Serial.print(", diff:");  
//      Serial.print(adj_factor_diff, 4);
//      Serial.print(", factor:");  
//      Serial.println(adj_factor, 4);
//      Serial.flush();
      v = v + adj_factor; 
      
      return v;
    }
  } 
  return milliVolt;  
}

void CalibrationClass::floatToEeprom(int address, float f) {
  Serial.print("Trying to convert float:");
  Serial.print(f,4);
  Serial.println(" to bytes for eeprom storage.");

  cvt eepromfloat;
  eepromfloat.val = f;

  EEPROM.write(address,eepromfloat.b[0]);
  EEPROM.write(address+1,eepromfloat.b[1]); 
  EEPROM.write(address+2,eepromfloat.b[2]); 
  EEPROM.write(address+3,eepromfloat.b[3]); 
  Serial.println(eepromfloat.b[0]);
  Serial.println(eepromfloat.b[1]);
  Serial.println(eepromfloat.b[2]);
  Serial.println(eepromfloat.b[3]);

}

float CalibrationClass::floatFromEeprom(int address) {
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
  Serial.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  Serial.print("Dac gain pos comp at address ");
  Serial.print(ea_dac_gain_comp_pos,HEX);
  Serial.print(" adjusted to:");
  Serial.println(dacGainCompPos,6);
  Serial.flush();
}

float CalibrationClass::getDacGainCompPos() {
  return dacGainCompPos;
}

void CalibrationClass::adjDacGainCompNeg(float val) {
  dacGainCompNeg += val;
  floatToEeprom(ea_dac_gain_comp_neg, dacGainCompNeg);
  Serial.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
   Serial.print("Dac gain neg comp at address ");
  Serial.print(ea_dac_gain_comp_neg,HEX);
  Serial.print(" adjusted to:");
  Serial.println(dacGainCompNeg,6);
  Serial.flush();
}

float CalibrationClass::getDacGainCompNeg() {
  return dacGainCompNeg;
}


void CalibrationClass::adjAdcGainCompPos(float val) {
  adcGainCompPos += val;
  floatToEeprom(ea_adc_gain_comp_pos, adcGainCompPos);
  Serial.print("Operation type = ");
  Serial.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  Serial.print("Adc gain pos comp at address ");
  Serial.print(ea_adc_gain_comp_pos,HEX);
  Serial.print(" adjusted to:");
  Serial.println(adcGainCompPos,6);
  Serial.flush();
}

float CalibrationClass::getAdcGainCompPos() {
  return adcGainCompPos;
}

void CalibrationClass::adjAdcGainCompNeg(float val) {
  adcGainCompNeg += val;
  floatToEeprom(ea_adc_gain_comp_neg, adcGainCompNeg);
  Serial.print("Operation type = ");
  Serial.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  Serial.print("Adc gain neg comp at address ");
  Serial.print(ea_adc_gain_comp_neg,HEX);
  Serial.print(" adjusted to:");
  Serial.println(adcGainCompNeg,6);
  Serial.flush();
}

float CalibrationClass::getAdcGainCompNeg() {
  return adcGainCompNeg;
}

void CalibrationClass::adjDacZeroComp(float val) {
  dacZeroComp += val;
  floatToEeprom(ea_dac_zero_comp, dacZeroComp);
  Serial.print("Operation type = ");
  Serial.println(operationType == SOURCE_CURRENT ? "Source current" : "Source voltage");
  Serial.print("Dac zero comp at address ");
  Serial.print(ea_dac_zero_comp,HEX);
  Serial.print(" adjusted to:");  
  Serial.println(dacZeroComp,6);

  Serial.flush();
}

float CalibrationClass::getDacZeroComp() {
  return dacZeroComp;
}



void CalibrationClass::renderCal(int x, int y, float valM, float setM, bool cur, bool reduceDetails) {
  if (!reduceDetails) {
    GD.ColorRGB(0xaaaaaa);
    if (FILTERS->mean < 0) {
      GD.cmd_text(x+10, y + 45, 27, 0, "DAC GAIN -");
      DIGIT_UTIL.renderValue(x + 0,  y+65 ,getDacGainCompNeg()*100.0, 1, -1); 
      GD.cmd_text(x+93, y + 68, 27, 0, "%");
    } else {
      GD.cmd_text(x+10, y + 45, 27, 0, "DAC GAIN +");
      DIGIT_UTIL.renderValue(x + 0,  y+65 ,getDacGainCompPos()*100.0, 1, -1);
      GD.cmd_text(x+93, y + 68, 27, 0, "%"); 
    }
  
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_DAC_GAIN_COMP_POS_UP);
    GD.cmd_button(x+10,y+90,100,50,29,0,"UP");
    GD.Tag(BUTTON_DAC_GAIN_COMP_POS_DOWN);
    GD.cmd_button(x+10,y+150,100,50,29,0,"DOWN");
  GD.Tag(0);
    x=x+10;
    GD.ColorRGB(0xaaaaaa);
    if (FILTERS->mean < 0) {
      GD.cmd_text(x+120, y + 45, 27, 0, "ADC GAIN -");
      DIGIT_UTIL.renderValue(x + 110,  y+65 ,getAdcGainCompNeg() * 100.0, 1, -1); 
      GD.cmd_text(x+203, y + 68, 27, 0, "%");
    } else {
      GD.cmd_text(x+120, y + 45, 27, 0, "ADC GAIN +");
      DIGIT_UTIL.renderValue(x + 110,  y+65 ,getAdcGainCompPos() * 100.0, 1, -1); 
      GD.cmd_text(x+203, y + 68, 27, 0, "%");
    }
  
    GD.ColorRGB(0x000000);
  
    GD.Tag(BUTTON_ADC_GAIN_COMP_POS_UP);
    GD.cmd_button(x+120,y+90,100,50,29,0,"UP");
    GD.Tag(BUTTON_ADC_GAIN_COMP_POS_DOWN);
    GD.cmd_button(x+120,y+150,100,50,29,0,"DOWN");
GD.Tag(0);

 x=x+120;
    GD.ColorRGB(0xaaaaaa);
    
      GD.cmd_text(x+120, y + 45, 27, 0, "DAC zero");
      DIGIT_UTIL.renderValue(x + 110,  y+65 ,getDacZeroComp(), 1, -1); 
      GD.cmd_text(x+203, y + 68, 27, 0, "");
   
    GD.ColorRGB(0x000000);
  
    GD.Tag(BUTTON_DAC_ZERO_COMP_UP);
    GD.cmd_button(x+120,y+90,100,50,29,0,"UP");
    GD.Tag(BUTTON_DAC_ZERO_COMP_DOWN);
    GD.cmd_button(x+120,y+150,100,50,29,0,"DOWN");
    
    GD.Tag(0); // Seems to fix problem with function calles when touching a special place on the screen... why ????


x=x-70;
    

    GD.ColorRGB(0xaaaaaa);

    GD.cmd_text(x+300,y+45,27,0,"ADC NullV(1A)");
    DIGIT_UTIL.renderValue(x + 300,  y+60 ,nullValueVol[0], 1, -1); 
    GD.cmd_text(x+450,y+45,27,0,"ADC NullV(10mA)");
    DIGIT_UTIL.renderValue(x + 450,  y+60 ,nullValueVol[1], 1, -1); 

    GD.cmd_text(x+300,y+45+60,27,0,"ADC NullC(1A)");
    DIGIT_UTIL.renderValue(x + 300,  y+120 ,nullValueCur[0], 1, -1); 
    GD.cmd_text(x+450,y+45+60,27,0,"ADC NullC(10mA)");
    DIGIT_UTIL.renderValue(x + 450,  y+120 ,nullValueCur[1], 1, -1); 

    
    GD.ColorRGB(0x000000);

    GD.Tag(BUTTON_DAC_NONLINEAR_CALIBRATE);
    GD.cmd_button(x+620,y+45,120,50,29,0,"AUTOCAL");

    GD.Tag(BUTTON_DAC_ZERO_CALIBRATE);
    GD.cmd_button(x+620,y+100,120,50,29,0,"ZEROCAL");
    
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
  y=y+30+10;
  // correction graph
  if (!reduceDetails) {
    for (int i=0;i<adc_cal_points;i++) {
        float diff = set_adc[i] - meas_adc[i];
//        Serial.print("adc_cal_points=");
//        Serial.print(adc_cal_points);
//        Serial.print(", min_set_value=");
//        Serial.print(min_set_value);
//        Serial.print("max_set_value=");
//        Serial.print(max_set_value);
//        Serial.print(", Cal points ");
//        Serial.print(adc_cal_points);
//        Serial.print(" - Rendering set ");
//        Serial.print(set_adc[i]);
//        Serial.print(" meas ");
//        Serial.println(meas_adc[i]);
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
  
}

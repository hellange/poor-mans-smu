#include "Calibration.h"
#include "Arduino.h"
#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include <EEPROM.h>
#include "eeprom_adr.h"

CalibrationClass V_CALIBRATION;
CalibrationClass C_CALIBRATION;

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

void CalibrationClass::init(OPERATION_TYPE operationType_) {
  operationType = operationType_;
  if (operationType == SOURCE_VOLTAGE) {
    // set value
    adc_cal_points = 37;
    Serial.println("Voltage measurement nonlinearity:");
    float_array_init(set_adc, adc_cal_points,  -12000.00, -11000.00, -10000.00, -9000.00, -8000.00, -7000.00, -6000.00, -5000.00, -4000.00, -3500.00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, -50.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 2000.00, 2500.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00, 12000.00);
    float_array_init(meas_adc, adc_cal_points, -11992.94, -10996.74, -10000.00, -8999.41, -7998.84, -6998.41, -5996.70, -4995.87, -3996.70, -3497.11, -2997.69, -2497.78, -1997.91, -0999.54, -499.96, -100.01, -50.00, 0.00, 50.06, 100.16, 250.34, 400.54, 500.66, 900.95, 1000.95, 2000.68, 2501.31, 3001.89, 4002.48, 5003.02, 6005.36, 7009.10, 8010.84, 9008.52, 10000.00, 10992.71, 12002.12);

    dac_cal_points = 26;
    Serial.println("Voltage source nonlinearty:");
    float_array_init(set_dac, dac_cal_points,  -11000.00, -10000.00, -5000.00, -4000.00, -3000.00, -2000.00, -1000.00, 0.00, 5.00, 50.00, 100.00, 500.00, 1000.00, 2000.00, 3000.00, 3500.00, 4000.00, 4200.00, 4500.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00);
    float_array_init(meas_dac, dac_cal_points, -11000.00, -10000.00, -5000.00, -3999.98, -2999.90, -2000.14, -1000.07, 0.00, 5.00, 49.99, 100.02, 500.00, 0999.98, 1999.97, 3000.00, 3500.20, 4000.08, 4200.06, 4500.05, 5000.04, 6000.01, 7000.17, 8000.12, 9000.10, 10000.00, 11000.00);
  
    ea_dac_gain_comp_pos = EA_DAC_GAIN_COMP_POS_VOL;
    ea_dac_gain_comp_neg = EA_DAC_GAIN_COMP_NEG_VOL;
    ea_adc_gain_comp_pos = EA_ADC_GAIN_COMP_POS_VOL;
    ea_adc_gain_comp_neg = EA_ADC_GAIN_COMP_NEG_VOL;
  } else {

        // set value
    adc_cal_points = 37;
    Serial.println("Current measurement nonlinearity:");
    float_array_init(set_adc, adc_cal_points,  -12000.00, -11000.00, -10000.00, -9000.00, -8000.00, -7000.00, -6000.00, -5000.00, -4000.00, -3500-00, -3000.00, -2500.00, -2000.00, -1000.00, -500.00, -100.00, -50.00, 0.00, 50.00, 100.00, 250.00, 400.00, 500.00, 900.00, 1000.00, 2000.00, 2500.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00, 11000.00, 12000.00);
    float_array_init(meas_adc, adc_cal_points, -11992.94, -10996.74, -10000.00, -8999.41, -7998.84, -6998.41, -5996.70, -4995.87, -3996.70, -3497.11, -2997.69, -2497.78, -1997.91, -0999.54, -499.96, -100.01, -50.00, 0.00, 50.06, 100.16, 250.34, 400.54, 500.66, 900.95, 1000.95, 2000.68, 2501.31, 3001.89, 4002.48, 5003.02, 6005.36, 7009.10, 8010.84, 9008.52, 10000.00, 10992.71, 12002.12);

    dac_cal_points = 11;
    Serial.println("Current source nonlinearity:");
    float_array_init(set_dac, dac_cal_points,   -500.00, -400.00, -300.00, -200.00, -100.00, 0.00, 100.00, 200.00, 300.00, 400.00, 500.00);
    float_array_init(meas_dac, dac_cal_points,  -500.00, -400.00, -300.00, -200.00, -100.00, 0.00, 100.00, 200.00, 300.00, 400.00, 500.00);

    ea_dac_gain_comp_pos = EA_DAC_GAIN_COMP_POS_CUR;
    ea_dac_gain_comp_neg = EA_DAC_GAIN_COMP_NEG_CUR;
    ea_adc_gain_comp_pos = EA_ADC_GAIN_COMP_POS_CUR;
    ea_adc_gain_comp_neg = EA_ADC_GAIN_COMP_NEG_CUR;
  }

  nullValue[0] = 0.0;
  nullValue[1] = 0.0;
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
  return nullValue[current_range] != 0.0;
}

void CalibrationClass::toggleNullValue(float v, CURRENT_RANGE current_range) {
  if (timeSinceLastChange + 100 > millis()){
    return;
  }
  if (nullValue[current_range] != 0.0) {
    nullValue[current_range] = 0.0;
  } else {
    nullValue[current_range] = v;
  } 
  Serial.print("setNull to:");
  Serial.println(v);
  timeSinceLastChange = millis();

}
// todo: change parameter name to mv ?
float CalibrationClass::adc_nonlinear_compensation(float v){

  if (!useCalibratedValues) {
    return v;
  }
  
  // Nonlinearity
  //TODO: Does it work for negative voltages ?
  for (int i=0;i<adc_cal_points -1; i++) {
    if (v > meas_adc[i] && v <= meas_adc[i+1]) {
      float adj_factor_low = set_adc[i] - meas_adc[i];
      float adj_factor_high = set_adc[i+1] - meas_adc[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = set_adc[i+1] - set_adc[i];
      float partWithinRange = ( (v-set_adc[i]) / range); // 0 to 1. Where then 0.5 is in the middle of the range 
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;
 /*
      Serial.print("meas:");  
      Serial.print(v, 3);
      Serial.print(", range:");  
      Serial.print(range, 3);
      Serial.print(", part:");  
      Serial.print(partWithinRange, 3);
      Serial.print(", factor:");  
      Serial.println(adj_factor, 3);

      Serial.flush();
       */
      v= v + adj_factor;
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
    if (v > meas_dac[i] && v <= meas_dac[i+1]) {
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
  Serial.flush();
}

float CalibrationClass::getAdcGainCompNeg() {
  return adcGainCompNeg;
}

void CalibrationClass::renderCal(int x, int y, float valM, float setM, bool cur, bool reduceDetails) {

  GD.ColorRGB(0xaaaaaa);
  GD.cmd_text(x+10, y + 50, 27, 0, "DAC GAIN");
  GD.ColorRGB(0x000000);

  GD.Tag(BUTTON_DAC_GAIN_COMP_POS_UP);
  GD.cmd_button(x+10,y+90,100,50,29,0,"UP");
  GD.Tag(BUTTON_DAC_GAIN_COMP_POS_DOWN);
  GD.cmd_button(x+10,y+150,100,50,29,0,"DOWN");

  GD.ColorRGB(0xaaaaaa);
  GD.cmd_text(x+120, y + 50, 27, 0, "ADC GAIN");
  GD.ColorRGB(0x000000);

  GD.Tag(BUTTON_ADC_GAIN_COMP_POS_UP);
  GD.cmd_button(x+120,y+90,100,50,29,0,"UP");
  GD.Tag(BUTTON_ADC_GAIN_COMP_POS_DOWN);
  GD.cmd_button(x+120,y+150,100,50,29,0,"DOWN");
  
  GD.LineWidth(20);
  GD.Begin(LINE_STRIP);
  GD.ColorA(255);
  GD.ColorRGB(0xff0000);

  float max_set_value = set_adc[adc_cal_points-1];
  float min_set_value = set_adc[0];

  float max_meas_value = meas_adc[adc_cal_points-1];

  float pixelsPrVolt = 200; // pixel width pr volt
  float x_null_position = 550; // x position for 0V
  float correction_display_factor = 100000.0; // TODO: Make it show as ppm ?  uV ?

  // correction graph
  if (!reduceDetails) {
    for (int i=0;i<adc_cal_points;i++) {
        float diff = set_adc[i] - meas_adc[i];
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

  // voltage labels
  if (!reduceDetails) {
  for (int i=min_set_value/1000.0;i<=max_set_value/1000.0;i=i+2) {
      int xv = i* pixelsPrVolt/(int)(max_set_value/1000.0);
      GD.cmd_text(x+x_null_position+xv-10, y + 200, 27, 0, i<0?"-":" ");
      if (abs(i) >=10.0) {
        GD.cmd_number(x+x_null_position+xv, y + 200, 27, 2, abs(i));
        GD.cmd_text(x+x_null_position+xv+20, y + 200, 27, 0, "V");
      } else {
        GD.cmd_number(x+x_null_position+xv, y + 200, 27, 1, abs(i));
        GD.cmd_text(x+x_null_position+xv+10, y + 200, 27, 0, "V");
      }
  }
  }
  
}

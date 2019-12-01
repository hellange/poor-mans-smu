#include "Calibration.h"
#include "Arduino.h"
#include <SPI.h>
#include "GD2.h"


CalibrationClass V_CALIBRATION;
CalibrationClass C_CALIBRATION;
  
void CalibrationClass::init() {
  nullValue = 0.0;
  timeSinceLastChange = millis();
 
}

bool CalibrationClass::toggleCalibratedValues() {
  if (timeSinceLastChange + 1000 > millis()){
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

void CalibrationClass::toggleNullValue(float v) {
  if (timeSinceLastChange + 1000 > millis()){
    return;
  }
  if (nullValue != 0.0) {
    nullValue = 0.0;
  } else {
    nullValue = v;
  } 
  Serial.print("setNull to:");
  Serial.println(v);
  timeSinceLastChange = millis();

}
// todo: change parameter name to mv ?
float CalibrationClass::adjust(float v){

  if (!useCalibratedValues) {
    return v;
  }
  
  // Nonlinearity
  //TODO: Does it wor for negative voltages ?
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

void CalibrationClass::renderCal(int x, int y, float valM, float setM, bool cur) {
        GD.LineWidth(20);
      GD.ColorA(150);
      GD.ColorRGB(0x666666);

      GD.Begin(LINE_STRIP);

      
//      for (int i=0;i<adc_cal_points -1; i++) {
//        GD.Vertex2ii(x+100+i*50, y + 200 - set_adc[i] / 10.0);
//        GD.Vertex2ii(x+100+i*50, - 10 +y + 200 - set_adc[i] / 10.0);
//        GD.Vertex2ii(x+100+i*50, y + 200 - set_adc[i] / 10.0);
//
//      }


      GD.Begin(LINE_STRIP);
      GD.ColorA(255);
      GD.ColorRGB(0xff0000);

      float max_set_value = set_adc[adc_cal_points-1];
            float min_set_value = set_adc[0];

      float max_meas_value = meas_adc[adc_cal_points-1];

  // correction graph
  float correction_display_factor = 100000.0; // TODO: Make it show as ppm ?  uV ?
  for (int i=0;i<adc_cal_points;i++) {
      float diff = set_adc[i] - meas_adc[i];
      int xv = 300 *(set_adc[i] / max_set_value);
      int yv = /*150 *(meas_adc[i] / max_meas_value) - */(diff/max_meas_value) * correction_display_factor;
      GD.Vertex2ii(x+400+xv, y + 100 - yv);
  }

 // voltage axis
 GD.ColorRGB(0x00ff00);
 GD.Begin(LINE_STRIP);
      GD.ColorA(100);
  for (int i=0;i<adc_cal_points;i++) {
      //float diff = set_adc[i] - meas_adc[i];
      int xv = 300 *(set_adc[i] / max_set_value);
      GD.Vertex2ii(x+400+xv, y + 100);
  }

  // voltage labels
  for (int i=min_set_value/1000.0;i<=max_set_value/1000.0;i++) {
      int xv = i* 300/(int)(max_set_value/1000.0);
      GD.cmd_text(x+400+xv-10, y + 200, 27, 0, i<0?"-":" ");
      GD.cmd_number(x+400+xv, y + 200, 27, 1, abs(i));
      GD.cmd_text(x+400+xv+10, y + 200, 27, 0, "V");

  }

  
  
      GD.ColorA(255);

      GD.ColorRGB(0xeeeeee);

//  for (int i=0;i<10;i++) {
//        
//      GD.cmd_number(x+100+i*50 - 15, y + 210, 26, 4, (int)set_adc[i]);
//
//  }
  
}

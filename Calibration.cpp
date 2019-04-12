#include "Calibration.h"
#include "Arduino.h"
#include <SPI.h>
#include "GD2.h"


//TODO: remove globals
  float real_c[]  = {0.0, 1000.00, 2000.00, 3000.00, 4000.00, 5000.00, 6000.00, 7000.00, 8000.00, 9000.00, 10000.00};
  float meas_c[]  = {0.0, 0999.25, 1998.25, 2997.24, 3999.90, 4999.95, 5994.08, 6999.84, 7999.82, 8990.94, 9990.05};

  
void CalibrationClass::init() {
}

float CalibrationClass::adjust(float v){

  // lookup table for nonlinearity. As many as possible.
  
  // offset
  v = v - 0.330 / 2.5;
  
  // system gain
  v = v * 2.5;  // account for attenuator

  // other gain factors
  v = v * 1.01589; 

  // Nonlinearity
  for (int i=0;i<10;i++) {
    if (v > meas_c[i] && v <= meas_c[i+1]) {
      float adj_factor_low = real_c[i] - meas_c[i];
      float adj_factor_high = real_c[i+1] - meas_c[i+1];
      float adj_factor_diff = adj_factor_high - adj_factor_low;

      float range = real_c[i+1] - real_c[i];
      float partWithinRange = ( (v-real_c[i]) / range); /* 0 to 1. Where then 0.5 is in the middle of the range */
      float adj_factor = adj_factor_low + adj_factor_diff * partWithinRange;
 
      Serial.print("meas:");  
      Serial.print(v, 3);
      Serial.print(", range:");  
      Serial.print(range, 3);
      Serial.print(", part:");  
      Serial.print(partWithinRange, 3);
      Serial.print(", factor:");  
      Serial.println(adj_factor, 3);

      Serial.flush();
       
      v= v + adj_factor;

    }
  }

  if (abs(v)>=100.0) {
       v = v;
     } else if (abs(v)>=50.0) {
       v = v *1.0005;
     } else {
       v = v * 1.003;
     }
  return v;
}

void CalibrationClass::renderCal(int x, int y, float valM, float setM, bool cur) {
        GD.LineWidth(20);
      GD.ColorA(150);
      GD.ColorRGB(0x666666);

      GD.Begin(LINE_STRIP);

      for (int i=0;i<10;i++) {
        GD.Vertex2ii(x+100+i*50, y + 200 - real_c[i] / 100.0);
        GD.Vertex2ii(x+100+i*50, - 10 +y + 200 - real_c[i] / 100.0);
        GD.Vertex2ii(x+100+i*50, y + 200 - real_c[i] / 100.0);

      }


      GD.Begin(LINE_STRIP);
      GD.ColorA(255);
      GD.ColorRGB(0xff0000);

  for (int i=0;i<10;i++) {
      float diff = real_c[i] - meas_c[i];
      GD.Vertex2ii(x+100+i*50, y + 200 - (meas_c[i]+diff*100.0) / 100.0);
  }

      GD.ColorRGB(0xeeeeee);

  for (int i=0;i<10;i++) {
        
      GD.cmd_number(x+100+i*50 - 15, y + 210, 26, 4, (int)real_c[i]);

  }
  
}





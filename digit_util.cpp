#include "digit_util.h"
#include <SPI.h>
#include "GD2.h"

// TODO: A possible error in this function.
//       I often see the uV area jump from low (0-100) to high (900-999) without
//       the mv changing... What's the precision in arduino calculations ?
void DigitUtilClass::separate(int *v, int *mv, int *uv, bool *neg, float rawMv) {
  *neg=false;
  if (rawMv < 0.0f) {
    rawMv = 0.0f - rawMv;
    *neg=true;
  }
  *v = (int)(rawMv / 1000.0f);
  *mv = (int)(rawMv - *v * 1000.0f);
  *uv = (rawMv - (int)rawMv) * 1000.0f;
}

void DigitUtilClass::separate2(int *v, int *mv, int *uv, int *nv, bool *neg, float rawMv) {
  float uv_f;
  *neg=false;
  if (rawMv < 0.0f) {
    rawMv = 0.0f - rawMv;
    *neg=true;
  }
  *v = (int)(rawMv / 1000.0f);
  *mv = (int)(rawMv - *v * 1000.0f);
  uv_f = (rawMv - (int)rawMv) * 1000.0f;
  *uv = uv_f;
  *nv = (uv_f - int(uv_f)) * 1000.0;
}

void DigitUtilClass::renderValue(int x,int y,float val, int size = 0, int type = 0) {

    int font = 26;
    int fontWidth = 10; 
    
    if (size == 1) {
       font = 28;
       fontWidth = 15;
    }
    else if (size == 2) {
       font = 29;
       fontWidth = 18;
    }
    else if (size == 3) {
       font = 30;
       fontWidth = 21;
    }
    else if (size == 4) {
       font = 31;
       fontWidth = 29;
    }
    
    int v, mV, uV, nV;
    bool neg;
    DIGIT_UTIL.separate2(&v, &mV, &uV, &nV, &neg, val);
    if(neg) {
      GD.cmd_text(x, y, font, 0,  "-");
    }

    x = x + fontWidth;
    if (v > 0) {
      GD.cmd_number(x, y, font, 2, v);
      x = x + fontWidth*1.7;
      GD.cmd_text(x, y, font, 0,  ".");
      x = x + fontWidth/3;
      GD.cmd_number(x, y, font, 3, mV);
      x = x + fontWidth * 2.9;
      GD.cmd_number(x, y, font, 3, uV);
      x = x + fontWidth * 2.6;
      GD.cmd_text(x, y, font, 0,  type == typeVoltage ? "V" : "A");
    } else if (mV > 0 or type == typeVoltage) {
      GD.cmd_number(x, y, font, 3, mV);
      x = x + fontWidth*2.5;
      GD.cmd_text(x-1, y, font, 0,  ".");
      x = x + fontWidth/3;
      GD.cmd_number(x, y, font, 3, uV);
      x = x + fontWidth * 2.6;
      GD.cmd_text(x, y, font, 0,  type == typeVoltage ? "mV" : "mA");
    } else {
      GD.cmd_number(x, y, font, 3, uV);
      x = x + fontWidth*2.5;
      GD.cmd_text(x-1, y, font, 0,  ".");
      x = x + fontWidth/3;
      GD.cmd_number(x, y, font, 3, nV);
      x = x + fontWidth * 2.6;
      GD.cmd_text(x, y, font, 0,  type == typeVoltage ? "uV" : "uA");
    }

}

DigitUtilClass DIGIT_UTIL;

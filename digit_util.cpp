#include "digit_util.h"

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
  //*uv = (int)((rawMv - *mv) * 1000.0f);
  *uv = (rawMv - (int)rawMv) * 1000.0f;

}
DigitUtilClass DIGIT_UTIL;


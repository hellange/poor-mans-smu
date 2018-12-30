#include "Stats.h"
#include "GD2.h"

void StatsClass::addSample(float rawMv) {
      value[endPtr] = rawMv;

      endPtr ++;
      if (endPtr > nrOfTrendPoints - 1) {
        endPtr = 0;
      } 
      minimum = 100000.0;  // +100V
      maximum = -100000.0; // -100V
      updateMinMax(rawMv);
    
      span = maximum - minimum;
}


void StatsClass::updateMinMax(float rawMv) {
    for (int i=0;i<nrOfTrendPoints;i++) {
        if (value[i]<=minimum) {
          minimum = value[i];
        }
        if (value[i]>=maximum) {
          maximum = value[i];
        }
      }
}

void StatsClass::renderTrend(int x, int y , bool small) {  

    
    int viewHeight;

    if (x>800) {
      return;
    }
    visibleMax = maximum;
    visibleMin = minimum;

    uispan = visibleMax - visibleMin;

    GD.Begin(LINE_STRIP);
    int multiplyBy = 1;
    if (!small) {
      multiplyBy = 7;
      GD.LineWidth(15);
      viewHeight = 150;
    } else {
      GD.LineWidth(4);
      viewHeight = 45;
    }
    
    int i = endPtr;
 
    for (int pos=0; pos<nrOfTrendPoints;pos++) { 

      float height = viewHeight * ( (visibleMax - value[i]) / uispan);

      int xpos = x + pos*(1+multiplyBy);
      if (xpos>800) {
        return;
      }
      GD.Vertex2ii(xpos, y + height); 
      i=i+1;
      if (i>nrOfTrendPoints - 1) {
        i=0;
      }  
    }

}



#include "Stats.h"
#include "GD2.h"

int samplesBeforeStore = 10000;

void StatsClass::addSample(float rawMv_) {
      rawMv = rawMv_; 

      samplesBeforeStore ++;
      if (samplesBeforeStore++ < 5) {
        return;
      } else {
        samplesBeforeStore = 0;
      }
      
      value[endPtr] = rawMv_;


      endPtr ++;
      if (endPtr > nrOfTrendPoints - 1) {
        endPtr = 0;
      } 
      minimum = 100000.0;  // +100V
      maximum = -100000.0; // -100V
      updateMinMax(rawMv_);
    
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

void StatsClass::renderTrend(int x, int y) {  
    
    int viewHeight;

    if (x>800) {
      return;
    }
    visibleMax = maximum;
    visibleMin = minimum;
    // dont show spans smaller that a given value
    float minimumVisibleSpan = 1.0;
    float span = maximum - minimum;
    if (span < minimumVisibleSpan) {
      visibleMax = visibleMax + (minimumVisibleSpan-span)/2.0;
      visibleMin = visibleMin - (minimumVisibleSpan-span)/2.0;
    }

    uispan = visibleMax - visibleMin;
    
    int multiplyBy = 7;
    viewHeight = 150;

    GD.Begin(LINE_STRIP);
    GD.LineWidth(30);

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

    float top = viewHeight * ( (visibleMax - maximum) / uispan);
    float bottom = viewHeight * ( (visibleMax - minimum) / uispan);
      GD.Begin(LINE_STRIP);
      GD.LineWidth(20);
      GD.ColorRGB(0xffff00);

      GD.Vertex2ii(x+5, y + top); 
      GD.Vertex2ii(x, y + top + 2); 
      GD.Vertex2ii(x, y + (viewHeight)/2 - 5); 
      GD.Vertex2ii(x-5, y + (viewHeight)/2 ); 
      GD.Vertex2ii(x, y + (viewHeight)/2 + 5); 
      GD.Vertex2ii(x, y + bottom - 2);
      GD.Vertex2ii(x+5, y + bottom); 

}



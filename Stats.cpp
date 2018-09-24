#include "Stats.h"
#include "GD2.h"

void StatsClass::addSample(float rawMv) {
  value[endPtr] = rawMv;

      endPtr ++;
      if (endPtr > nrOfTrendPoints - 1) {
        endPtr = 0;
      } 
      minimum = value[0];
      maximum = value[0];

      // Calculate max and min
      for (int i=0;i<nrOfTrendPoints;i++) {
        if (value[i]<minimum) {
          minimum = value[i];
        }
          if (value[i]>maximum) {
          maximum = value[i];
        }
      }
      span = maximum - minimum;
}

void StatsClass::renderTrend(int x, int y , int samples, bool small) {  

    nrOfTrendPoints = samples;
    // set a mimimum span so very small changes in uV
    // dont show up as full span. Dont need to look
    // more noisy that it is... Adjustable ?
    float minVisibleSpan = 0.100f;
    
    float diff = maximum - minimum;
    visibleMax = maximum;
    visibleMin = minimum;
    if (diff < minVisibleSpan) {
      visibleMax = maximum + ((minVisibleSpan - diff)/2.0f);
      visibleMin = minimum - ((minVisibleSpan - diff)/2.0f);
    }
    uispan = visibleMax - visibleMin;

    int multiplyBy = 1;
    if (!small) {
      multiplyBy = 2;
    }
    
    int i = endPtr;
    GD.Begin(LINE_STRIP);
    GD.LineWidth(multiplyBy * 10);
 
    for (int pos=0; pos<nrOfTrendPoints;pos++) { 

      float height = 45.0f * ( (visibleMax - value[i]) / uispan);
      if (!small) {
        height=height*multiplyBy;
      }
      GD.Vertex2ii(x + pos*(1+multiplyBy), y + height); 
      i=i+1;
      if (i>nrOfTrendPoints - 1) {
        i=0;
      }  
    }

}



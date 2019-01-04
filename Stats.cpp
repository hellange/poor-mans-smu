#include "Stats.h"
#include "GD2.h"



void StatsClass::addSample(float rawMv_) {
      rawMv = rawMv_; 
      prelimBuffer[prelimSamplesCounter++] = rawMv_;
      
      if (prelimSamplesCounter < maxSamplesBeforeStore) {
        return;
      } else {
        valueExt[endPtr][0] = prelimBuffer[0];
        valueExt[endPtr][1] = prelimBuffer[0];
        float tot=prelimBuffer[0];
        for (int i=1; i< maxSamplesBeforeStore ; i++) {
          tot = tot + prelimBuffer[i];
          if (prelimBuffer[i] <= valueExt[endPtr][0]) {
            valueExt[endPtr][0] = prelimBuffer[i];   // stor as min value
           }
          if (prelimBuffer[i] > valueExt[endPtr][1]) {
            valueExt[endPtr][1] = prelimBuffer[i];   // stor as max value
          }
          
        }
        value[endPtr] = tot/maxSamplesBeforeStore; // set value as mean value of prelim samples

        prelimSamplesCounter = 0;
        updateMinMax();
        span = maximum - minimum;

      }
      
      endPtr ++;
      if (endPtr > nrOfTrendPoints - 1) {
        endPtr = 0;
      } 

}


void StatsClass::updateMinMax() {      
    minimum = 100000.0;
    maximum = -minimum;
    for (int i=0;i<nrOfTrendPoints;i++) {
        if (valueExt[i][0]<=minimum) {
          minimum = valueExt[i][0];
        }
        if (valueExt[i][1]>=maximum) {
          maximum = valueExt[i][1];
        }
      }
}


float visibleMaxSlow = -100000, visibleMinSlow = 100000;
void StatsClass::renderTrend(int x, int y) {  
    
    int viewHeight;
    GD.ColorA(255);

    // initially, let's say highest value in graph corresponds to highest value in sample set
    visibleMax = maximum;
    // and lowest graph value corresponds to lowest value in sample set
    visibleMin = minimum;

    // Try to introduce slow transition of graph highest value when highest sample value suddenly drops 
    if (maximum >= visibleMaxSlow) {
       visibleMaxSlow = maximum;
    } else {
       visibleMaxSlow = visibleMaxSlow - (visibleMaxSlow - maximum)/3.0;
      if (visibleMaxSlow >= maximum) {
         visibleMax = visibleMaxSlow;
      }
    }

     // Try to introduce slow transition of graph lowest value when lowest value suddenly increases
    if (minimum <= visibleMinSlow) {
       visibleMinSlow = minimum;
    } else {
       visibleMinSlow = visibleMinSlow + (minimum - visibleMinSlow)/3.0;   
       if (visibleMinSlow < minimum) {
          visibleMin = visibleMinSlow;
       }         
    }    

    // set a minimum full span view so very small value changes will not appear too noisy
    float minimumVisibleSpan = 0.750; 
    float span = maximum - minimum;
    if (span < minimumVisibleSpan) {
      visibleMax = visibleMax + (minimumVisibleSpan-span)/2.0;
      visibleMin = visibleMin - (minimumVisibleSpan-span)/2.0;
    }

    uispan = visibleMax - visibleMin;

    int pixelsPrSample = 7;
    viewHeight = 150;

    GD.Begin(LINE_STRIP);
    GD.LineWidth(30);

    int i = endPtr;

 

    // main graph
    for (int pos=0; pos<nrOfTrendPoints;pos++) { 
      float height = viewHeight * ( (visibleMax - value[i]) / uispan);
      int xpos = x + pos*(1+pixelsPrSample);
      if (xpos>800) {
        return;
      }
      GD.Vertex2ii(xpos, y + height); 
      i=i+1;
      if (i>nrOfTrendPoints - 1) {
        i=0;
      }  
    }

    i = endPtr;

    // max min indicator
    GD.ColorA(150);
    GD.LineWidth(50);
    for (int pos=0; pos<nrOfTrendPoints;pos++) { 
      float min = viewHeight * ( (visibleMax - valueExt[i][0]) / uispan);
      float max = viewHeight * ( (visibleMax - valueExt[i][1]) / uispan);
      int xpos = x + pos*(1+pixelsPrSample);
      if (xpos>800) {
        return;
      }
      GD.Begin(LINE_STRIP);
      GD.Vertex2ii(xpos, y + min); 
      GD.Vertex2ii(xpos, y + max); 

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



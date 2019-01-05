#include "Stats.h"
#include "GD2.h"
#include "digit_util.h"

void StatsClass::init() {
      for (int pos=0; pos<nrOfTrendPoints;pos++) { 
        value[pos] = 1000000;
      }
}
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
void StatsClass::renderTrend(int x, int y, bool limitDetails) {  
    int viewHeight;
    GD.ColorA(255);

// Add by 1000 to reduce decimal roudning problems.... should have stored value in uV and uA instead of mV and mA....
maximum = (int)(maximum *1000.0);
minimum = (int)(minimum *1000.0);

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


 
/*
    // round the max and min to get more "round" values for top and bottom in graph
    int visibleMaxInt;
    int visibleMinInt;
    float roundDecimals = 10.0;
    visibleMaxInt = (int)(visibleMax * roundDecimals);
    visibleMax = ((float)(visibleMaxInt +1))/roundDecimals;
    
    visibleMinInt = (int)(visibleMin * roundDecimals);
    visibleMin = ((float)(visibleMinInt-1))/roundDecimals;
*/

    // set a minimum full span view so very small value changes will not appear too noisy
    float minimumVisibleSpan = 750; //0.750; 
    float span = maximum - minimum;
    if (span < minimumVisibleSpan) {
      visibleMax = visibleMax + (minimumVisibleSpan-span)/2.0;
      visibleMin = visibleMin - (minimumVisibleSpan-span)/2.0;
    }
    

  //  Add by 1000 to reduce decimal roudning problems.... should have stored value in uV and uA instead of mV and mA....
  maximum = maximum/1000.0;
  minimum = minimum/1000.0;

  // round the max and min to get more "round" values for top and bottom in graph
  visibleMax = ((int)(visibleMax/10.0) + 1) * 10;     
  visibleMax = visibleMax/1000.0;
  visibleMin = ((int)(visibleMin/10.0)) * 10;     
  visibleMin = visibleMin/1000.0;


    // set a minimum full span view so very small value changes will not appear too noisy
    /*
    float minimumVisibleSpan = 0.750; 
    float span = maximum - minimum;
    if (span < minimumVisibleSpan) {
      visibleMax = visibleMax + (minimumVisibleSpan-span)/2.0;
      visibleMin = visibleMin - (minimumVisibleSpan-span)/2.0;
    }
    */



    uispan = visibleMax - visibleMin;

    int pixelsPrSample = 6;
    
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
      if (value[i] < 1000000){
        GD.Vertex2ii(xpos, y + height); 
      }
      i=i+1;
      if (i>nrOfTrendPoints - 1) {
        i=0;
      }  
    }

    i = endPtr;

    // max min graphing
    if (!limitDetails) {

      GD.LineWidth(40);
      GD.ColorA(150);
  
      for (int pos=0; pos<nrOfTrendPoints;pos++) { 
        float min = viewHeight * ( (visibleMax - valueExt[i][0]) / uispan);
        float max = viewHeight * ( (visibleMax - valueExt[i][1]) / uispan);
        int xpos = x + pos*(1+pixelsPrSample);
        if (xpos>800) {
          return;
        }
        if (value[i] < 1000000){
          GD.Begin(LINE_STRIP);
          // red color if difference between mean value and extremes are > 1%
          if (valueExt[i][0] < value[i]*0.99 || valueExt[i][1] > value[i]*1.01){
             GD.ColorRGB(0xff0000); // red
          } else {
             GD.ColorRGB(0x00F94E); // COLOR_VOLT
          }
  
          GD.Vertex2ii(xpos, y + min); 
          GD.Vertex2ii(xpos, y + max); 
        }
        i=i+1;
        if (i>nrOfTrendPoints - 1) {
          i=0;
        }  
      }
    }
    

    float top = viewHeight * ( (visibleMax - maximum) / uispan);
    float bottom = viewHeight * ( (visibleMax - minimum) / uispan);

    GD.ColorRGB(0xffff00);
    GD.ColorA(255);

    DIGIT_UTIL.renderValue(x-10, y + top - 18, maximum, 0); 

    GD.Begin(LINE_STRIP);
    GD.LineWidth(20);

    GD.Vertex2ii(x+5, y + top); 
    GD.Vertex2ii(x, y + top + 2); 
    GD.Vertex2ii(x, y + (viewHeight)/2 - 5); 
    GD.Vertex2ii(x-5, y + (viewHeight)/2 ); 
    GD.Vertex2ii(x, y + (viewHeight)/2 + 5); 
    GD.Vertex2ii(x, y + bottom - 2);
    GD.Vertex2ii(x+5, y + bottom);

    // if close to bottom, put it at the bottom, trying to avoid "flicker" when minimum close to the bottom
    if (viewHeight - bottom > 5) {
      DIGIT_UTIL.renderValue(x-10, y + bottom + 5, minimum, 0); 
    } else {
       DIGIT_UTIL.renderValue(x-10,  y + viewHeight + 5, minimum, 0); 
    }

    float actualSpan = maximum - minimum;
    //x=x-(actualSpan>=1000?2:0);
    GD.cmd_text(x-70+6, y - 25 + 12 + (viewHeight)/2, 26, 0, "Span");
    DIGIT_UTIL.renderValue(x-98, y +4 + (viewHeight)/2, actualSpan, 0); 
}



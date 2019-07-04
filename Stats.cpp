#include "Stats.h"
#include <SPI.h>
#include "GD2.h"
#include "digit_util.h"
#include "colors.h"

void StatsClass::init(int type_) {
   type = type_;
   for (int pos=0; pos<nrOfTrendPoints;pos++) { 
     value[pos] = undefinedValue;
     valueExt[pos][0] = undefinedValue;
     valueExt[pos][1] = undefinedValue;

   }
}


void StatsClass::addSample(float rawValue_) {
      rawValue = rawValue_;
      prelimBuffer[prelimSamplesCounter++] = rawValue_;
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

    // based on mean value of a few samples
    minimumMean = undefinedValue;
    maximumMean = -minimumMean;
    for (int i=0;i<nrOfTrendPoints;i++) {
      if (value[i]<=minimumMean && value[i]!=undefinedValue) {
        minimumMean = value[i];
      }
      if (value[i]>=maximumMean && value[i]!=undefinedValue) {
        maximumMean= value[i];
      }
    }
    

    // based on raw samples
    minimum = undefinedValue;
    maximum = -minimum;
    for (int i=0;i<nrOfTrendPoints;i++) {
      if (valueExt[i][0]<=minimum && valueExt[i][0]!=undefinedValue) {
        minimum = valueExt[i][0];
      }
      if (valueExt[i][1]>=maximum && valueExt[i][1]!=undefinedValue) {
        maximum = valueExt[i][1];
      }
    }
}

int StatsClass::getNrOfSamplesBeforeStore() {
  return maxSamplesBeforeStore;
}
void StatsClass::setNrOfSamplesBeforeStore(int s) {
  if (s > MAX_PRELIMBUFFER_SIZE) {
    s = MAX_PRELIMBUFFER_SIZE;  //TODO: add warning ?
  }
  maxSamplesBeforeStore = s;
}


void StatsClass::renderHistogram(int x, int y, bool limitDetails) {   
    //GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
    //GD.ColorRGB(0xdddddd);
    //GD.ColorA(255);
    //GD.cmd_text(x+26, y, 29, 0, type==DigitUtilClass::typeVoltage?"VOLTAGE HISTOGRAM":"CURRENT HISTOGRAM");

    uispan = visibleMax - visibleMin;

    int noOfbins = noOfBins;
    float bins[noOfBins];
    float binLimits[noOfBins + 1];
    float binMax = 0;
    float span = maximum - minimum;
    
    for (int i=0;i<noOfbins;i++){
      bins[i] = 0;
      binLimits[i] = minimum + (span/noOfbins)*i;
    }
    binLimits[noOfbins] = maximum *2;  // make sure last bin is large enough...

    for (int pos=0; pos<nrOfTrendPoints;pos++) { 
      if (value[pos] == undefinedValue) {
        break;
      }
      for(int i=0;i<noOfbins;i++){
        if (value[pos] >= binLimits[i] && value[pos] < binLimits[i+1]){
          bins[i]++; 
          if (bins[i]>=binMax) {
            binMax = bins[i];
          }
        }
      }
    }
    y=y+170;
    GD.LineWidth(30);
    GD.ColorA(255);
    // main graph
    int barWidth = 600 / noOfBins;
    for (int i=0;i<noOfbins;i++) { 
      int xpos = 100 + x + i * barWidth;
      if (xpos>800) {
        return;
      }
      GD.Begin(RECTS);
      GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
      GD.LineWidth(40);
      float top = (bins[i]/binMax)*100;      
      GD.Vertex2ii(xpos + 4, y); 
      GD.Vertex2ii(xpos+barWidth - 4, y - top); 

      GD.ColorRGB(0xffffff);
      if (!limitDetails) {
        GD.cmd_number(xpos-2+barWidth/2, y-top-25, 26, 0, bins[i]);
      }

      if (i==0 || i==((noOfbins-1)/2) || i==noOfbins-1){
        DIGIT_UTIL.renderValue(xpos-35, y+20, binLimits[i], 0, type);
        GD.LineWidth(10);
        GD.Begin(LINE_STRIP);
        GD.Vertex2ii(xpos, y+7); 
        GD.Vertex2ii(xpos, y+15); 

      }

    }

}


void StatsClass::renderTrend(int x, int y, bool limitDetails) {     
    //GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
    //GD.ColorRGB(0xdddddd);
    //GD.cmd_text(x+20, y, 29, 0, type==DigitUtilClass::typeVoltage?"VOLTAGE TREND":"CURRENT TREND");

    y=y+35;
    int lines = 11;
    int height = 150;
    float visibleSpan = visibleMax - visibleMin;
    float distanceBetweenLines = visibleSpan/(lines-1);

    // show values for the horisontal lines
    GD.ColorRGB(0xffffff);
    for (int i=0;i<lines;i=i+2) {
      if (i==0 || i ==lines-1) {
        GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
      } else {
        GD.ColorRGB(0xffffff);
      }
      DIGIT_UTIL.renderValue(x, 15 + y + i*height/(lines-1), visibleMax - distanceBetweenLines*i, 0, type);
    }


    // draw horisontal lines
    int farRight = x + 690;
    if (farRight > 780) {
      farRight = 780;
    }
    GD.LineWidth(8);
    for (int i=0;i<lines;i=i+2) {
      int yaxis = 23 + y + i*height/(lines - 1);
      GD.Begin(LINE_STRIP);
            if (i == 0 || i==5 || i==10) {
               GD.ColorRGB(0xbbbbbb);
            } else {
               GD.ColorRGB(0x666666);
            }

      GD.Vertex2ii(x+90, yaxis); 
      GD.Vertex2ii(farRight, yaxis); 
    }
   

    int viewHeight;

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

    // set a minimum full span view so very small value changes will not appear too noisy
    float minimumFullpan = 100.0; // 100u 
    float span = maximum - minimum;
    if (span < minimumFullpan) {
      float adjust= (minimumFullpan-span)/4.0;
      visibleMax = visibleMax + adjust;
      visibleMin = visibleMin - adjust;
    }    
  
    //  Add by 1000 to reduce decimal rounding problems.... should have stored value in uV and uA instead of mV and mA....
    maximum = maximum/1000.0;
    minimum = minimum/1000.0;
  
    // round the max and min to get more "round" values for top and bottom in graph
    visibleMax = ((int)(visibleMax/10.0) + 1) * 10;     
    visibleMax = visibleMax/1000.0;
    visibleMin = ((int)(visibleMin/10.0)) * 10;     
    visibleMin = visibleMin/1000.0;


    int pixelsPrSample = 7;
    
    viewHeight = 150;

    GD.Begin(LINE_STRIP);
    GD.LineWidth(30);
    GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
 
    GD.ColorA(255);

    x = x + 95;
    y = y + 23;
  
    int i = endPtr;

    uispan = visibleMax - visibleMin;
 if (!limitDetails) {
    // main graph
    for (int pos=0; pos<nrOfTrendPoints;pos++) { 
      float height = viewHeight * ( (visibleMax - value[i]) / uispan);
      int xpos = 5 + x + pos*(1+pixelsPrSample);
      if (xpos>800) { // avoid writing outside screen
        return; 
      }
      if (value[i] < undefinedValue){
        GD.Vertex2ii(xpos, y + height); 
      }
      i=i+1;
      if (i>nrOfTrendPoints - 1) {
        i=0;
      }  
    }

    i = endPtr;
 }
    if (!limitDetails) {
      
      // max min graphing
      GD.LineWidth(40);
      GD.ColorA(150);

      for (int pos=0; pos<nrOfTrendPoints;pos++) { 
        float min = viewHeight * ( (visibleMax - valueExt[i][0]) / uispan);
        float max = viewHeight * ( (visibleMax - valueExt[i][1]) / uispan);
        int xpos = 5 + x + pos*(1+pixelsPrSample);
        if (xpos>800) {
          return;
        }
        if (value[i] < undefinedValue){
          GD.Begin(LINE_STRIP);
          // red color if difference between mean value and extremes are > 1%
          //if (valueExt[i][0] < value[i]*0.99 || valueExt[i][1] > value[i]*1.01){
          //   GD.ColorRGB(0xff0000); // red
          //} else {
          //   GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
          //}
  
          GD.Vertex2ii(xpos, y + min); 
          GD.Vertex2ii(xpos, y + max); 
        }
        i=i+1;
        if (i>nrOfTrendPoints - 1) {
          i=0;
        }  
      }
    }
    
    x=x+600;
        if (!limitDetails) {

    // show the span info with top and bottom
    float top = viewHeight * ( (visibleMax - maximum) / uispan);
    float bottom = viewHeight * ( (visibleMax - minimum) / uispan);
    GD.ColorRGB(0xdddddd);
    GD.ColorA(255);
    DIGIT_UTIL.renderValue(x-125, y + top - 25, maximum, 1, type); 
    GD.Begin(LINE_STRIP);
    GD.LineWidth(20);
    GD.Vertex2ii(x, y + top); 
    GD.Vertex2ii(x+5, y + top + 2); 
    //GD.Vertex2ii(x, y + (viewHeight)/2 - 5); 
    //GD.Vertex2ii(x-5, y + (viewHeight)/2 ); 
    //GD.Vertex2ii(x, y + (viewHeight)/2 + 5); 
    GD.Vertex2ii(x+5, y + bottom - 2);
    GD.Vertex2ii(x, y + bottom);
    // if close to bottom, put it at the bottom, trying to avoid "flicker" when minimum close to the bottom
    if (viewHeight - bottom > 5) {
      DIGIT_UTIL.renderValue(x-125, y + bottom + 0, minimum, 1, type); 
    } else {
      DIGIT_UTIL.renderValue(x-125,  y + viewHeight + 0 , minimum, 1, type); 
    }
    float actualSpan = maximum - minimum;
    float actualSpan1 = maximumMean - minimumMean;
    int spanTextY = y + top + (bottom-top)/2;
    GD.ColorA(180);
    GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
    GD.cmd_text(x + 15, spanTextY -30 , 26, 0, "Span noise");
    GD.ColorA(255);
    GD.ColorRGB(0xffffff);
    DIGIT_UTIL.renderValue(x + 10,  spanTextY -15, actualSpan, 0 , type);
    GD.ColorA(255);
    GD.ColorRGB(type==DigitUtilClass::typeVoltage?COLOR_VOLT:COLOR_CURRENT);
    GD.cmd_text(x + 15, spanTextY + 5 , 26, 0, "Span signal");
    GD.ColorA(255);
    GD.ColorRGB(0xffffff);
    DIGIT_UTIL.renderValue(x + 10,  spanTextY + 20, actualSpan1, 0 , type); 
        }
}

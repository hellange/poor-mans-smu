#include "analogGauge.h"
#include <SPI.h>
#include "GD2.h"
#include "colors.h"

void AnalogGaugeClass::renderAnalogGauge(int x, int y, int width, float degrees, float value, const char *title) {
  // Show scale indicators
  int gaugeRadius = width/2;
  for (int i=-50; i<=50; i=i+10) {
      int needleColor;
      if (i==30 || i ==-30) {
        needleColor = COLOR_ORANGE;
      }
      else if (i>-40 && i<40) {
        needleColor = 0xffffff;
      } else {
        needleColor = 0xff0000;
      }
      showAnalogPin(x+gaugeRadius, y+gaugeRadius+10, gaugeRadius, gaugeRadius - 10, i, needleColor, gaugeRadius/4, false);
  }

  GD.Begin(LINE_STRIP);
  GD.ColorRGB(0x888888);
  GD.ColorA(255);
  GD.LineWidth(10);

  if (x+gaugeRadius < 800) {
    GD.Vertex2ii(x, y);
    GD.Vertex2ii(x, y+gaugeRadius+10);
    GD.Vertex2ii(x+gaugeRadius*2, y+gaugeRadius+10);
    GD.Vertex2ii(x+gaugeRadius*2, y);
    GD.Vertex2ii(x, y);
    
    showAnalogPin(x+gaugeRadius, y+gaugeRadius+10, gaugeRadius, 30, degrees, 0xffffff, 20, true);

    GD.Begin(RECTS);
    GD.ColorRGB(0x223322);
    GD.ColorA(255);
    GD.Vertex2ii(x+4, y+gaugeRadius-25 + 2);
    GD.Vertex2ii(x+gaugeRadius*2-4, y+gaugeRadius+10-4);
  
    GD.ColorRGB(0xffffff);
    GD.cmd_text(x+gaugeRadius/2, y-20, 27, 0, title);
  
    GD.ColorRGB(0xdddddd);
  }
  
  
}

void AnalogGaugeClass::renderAnalogGaugeValue(int x, int y, int width, float degrees, float value, const char *unit, const char *title) {
  renderAnalogGauge(x,y,width,degrees,value,title);
  int gaugeRadius = width/2;
  y=y+gaugeRadius-22;
  x=x+gaugeRadius*1.2/2;
  x=x-10;
  int font = 29;
    GD.cmd_number(x+30, y, font, 5, value);
    GD.cmd_text(x+100, y, font, 0, unit);

}

void AnalogGaugeClass::renderAnalogGaugePercent(int x, int y, int width, float degrees, float value, const char *title) {
  renderAnalogGauge(x,y,width,degrees,value,title);
  
  int gaugeRadius = width/2;
  y=y+gaugeRadius-22;
  x=x+gaugeRadius*1.2/2;
  float deviationInPercent = abs(value);

  int font = 29;
  if (deviationInPercent < 1.0) {
    GD.cmd_text(x+8, y, font, 0, "0.");
    GD.cmd_number(x+30, y, font, 3, deviationInPercent * 1000.0);
    GD.cmd_text(x+85, y, font, 0, "%");
  } else if (deviationInPercent > 10.0){
    GD.ColorRGB(255,0,0); // RED
    GD.cmd_text(x+25, y, font, 0, ">10%");
  } else if (deviationInPercent >= 1.0 && deviationInPercent <10.0){
    int whole = (int)deviationInPercent;
    GD.cmd_number(x+5, y, font, 1, whole );
    GD.cmd_text(x+20, y, font, 0, ".");
    GD.cmd_number(x+30, y, font, 2, (deviationInPercent - (float)whole) * 100.0);
    GD.cmd_text(x+65, y, font, 0, "%");
  }
}




  void AnalogGaugeClass::showAnalogPin(int x, int y, int radius, int radiusStart, int degreeRelativeToTop, int needleColor, int lineWidth, bool needle) {

  int maxDegree = 60; 
  
  degreeRelativeToTop = degreeRelativeToTop <-maxDegree ? -maxDegree: degreeRelativeToTop;
  degreeRelativeToTop = degreeRelativeToTop > maxDegree ? maxDegree : degreeRelativeToTop;
  
  float oneDegreeRad = 2*3.1415 / 360.0;
  float rad = (3.1415/2.0) - degreeRelativeToTop * oneDegreeRad;
  
  GD.ColorRGB(needleColor);
  GD.ColorA(255);
  GD.Begin(LINE_STRIP);
  GD.LineWidth(lineWidth);
  GD.Vertex2ii(x+cos(rad)*radius, y-sin(rad)*radius);
  GD.Vertex2ii(x+cos(rad)*radiusStart, y-sin(rad)*radiusStart);

  if (needle){
    GD.Vertex2ii(x+cos(rad)*radius, y-sin(rad)*radius);
    GD.Vertex2ii(x+cos(rad*1.04)*radiusStart, y-sin(rad*1.04)*radiusStart);
    GD.Vertex2ii(x+cos(rad*0.96)*radiusStart, y-sin(rad*0.96)*radiusStart);
    GD.Vertex2ii(x+cos(rad)*radius, y-sin(rad)*radius);
  }
}

AnalogGaugeClass ANALOG_GAUGE;

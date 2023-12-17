#include "widgets.h"



void WidgetsClass::init(SMU_HAL &SMU) {
   SMU1 = &SMU;
}

bool WidgetsClass::isScrolling() {
  return scrollDir != 0;
}

void WidgetsClass::showWidget(int y, int widgetNo, int scroll, FUNCTION_TYPE functionType, OPERATION_TYPE operationType, bool hasCompliance, bool reduceDetails, float getSetValue_micro, float limitValue_micro, CURRENT_RANGE currentRange, bool anyDialogOpen) {
  int yPos = y-6;
  if (widgetNo == 0 && functionType == SOURCE_SWEEP) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_VOLT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE VOLTAGE");
     }
     measureCurrentPanel(scroll, yPos + 20, hasCompliance, true, currentRange, limitValue_micro, reduceDetails);

  } else if (widgetNo == 0 && functionType == SOURCE_PULSE) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_CURRENT_TEXT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE CURRENT");
     }
     measureCurrentPanel(scroll, yPos + 20, hasCompliance, true, currentRange, limitValue_micro, reduceDetails);
  } else if (widgetNo ==0 && functionType==MEASURE_RESISTANCE) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_VOLT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE RESISTANCE");
     }
     measureResistancePanel(scroll, yPos + 20, hasCompliance, currentRange, limitValue_micro);
  } else if (widgetNo ==0 && operationType == SOURCE_VOLTAGE) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_CURRENT_TEXT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE CURRENT");
     }
     measureCurrentPanel(scroll, yPos + 20, hasCompliance, true, currentRange, limitValue_micro, reduceDetails);
  } else if (widgetNo ==0 && operationType == SOURCE_CURRENT) {
     if (scroll ==0){
       GD.ColorRGB(COLOR_VOLT);
       GD.cmd_text(20, yPos, 29, 0, "MEASURE VOLTAGE");
     }
     measureVoltagePanel(scroll, yPos + 20, hasCompliance, limitValue_micro, reduceDetails);
  }  else if (widgetNo == 1) {
    if (!anyDialogOpen){
       if (scroll ==0){
         GD.ColorRGB(COLOR_CURRENT_TEXT);
         GD.cmd_text(20, yPos, 29, 0, "CURRENT TREND");
       }
       renderCurrentGraph(scroll, yPos, reduceDetails);
    }
  } else if (widgetNo == 2) {
      if (!anyDialogOpen){
        if (scroll ==0){
          GD.ColorRGB(COLOR_VOLT);
          GD.cmd_text(20, yPos, 29, 0, "VOLTAGE TREND");
        }
        renderVoltageGraph(scroll, yPos, reduceDetails);
      }
  } else if (widgetNo == 3) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "VOLTAGE HISTOGRAM");
      }
      renderHistogram(scroll, yPos, reduceDetails);
  } else if (widgetNo == 4) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "EXPERIMENTAL");
      }

      if (operationType == SOURCE_VOLTAGE) {
        float rawM = V_FILTERS.mean;
        float setM = getSetValue_micro/1000.0;
        renderExperimental(scroll,yPos, rawM, setM, false, reduceDetails, currentRange, anyDialogOpen);
      } else {
        float rawM = C_FILTERS.mean;
        float setM = getSetValue_micro/1000.0;
        renderExperimental(scroll,yPos, rawM, setM, false, reduceDetails, currentRange, anyDialogOpen);
      }
  } 
  else if (widgetNo == 5) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "CAL");
      }

      if (operationType == SOURCE_VOLTAGE) {
         float rawM = V_FILTERS.mean;
         float setM = getSetValue_micro/1000.0;
         if (abs(setM) > 2300) {   // TODO: Hack, using MILLIAMP10 to indicate high volt range (10V). Stupid. Fix !!!!
            V_CALIBRATION.renderCal(scroll,yPos, rawM, setM, MILLIAMP10, reduceDetails);
         } else {
           // TODO: Hack, using MILLIAMP10 to indicate high volt range (10V). Stupid. Fix !!!!
            V_CALIBRATION.renderCal(scroll,yPos, rawM, setM, AMP1, reduceDetails);

         }
       } else {
         float rawM = C_FILTERS.mean;
        float setM = getSetValue_micro*1000.0;
        C_CALIBRATION.renderCal(scroll,yPos, rawM, setM, currentRange,  reduceDetails);
      }
      
  }
  else if (widgetNo == 6) {
      if (scroll ==0){
        GD.ColorRGB(COLOR_VOLT);
        GD.cmd_text(20, yPos, 29, 0, "CAL 2");
      }

      if (operationType == SOURCE_VOLTAGE) {
         float rawM = V_FILTERS.mean;
         float setM = getSetValue_micro/1000.0;
         V_CALIBRATION.renderCal2(scroll,yPos, rawM, setM, currentRange, reduceDetails);
       } else {
         float rawM = C_FILTERS.mean;
         float setM = getSetValue_micro/1000.0;
         C_CALIBRATION.renderCal2(scroll,yPos, rawM, setM, currentRange, reduceDetails);
      }   
  }
  

}

void WidgetsClass::renderBar(int x, int y, float rawValue, uint64_t setValue_u, bool reduceDetails) {
  float setValue = setValue_u/1000.0; // logic below based on milli
  if (reduceDetails) {
    return;
  }
  GD.Begin(RECTS);
  GD.LineWidth(10);
  for (int i=0;i<40;i++) {
    int percent = 100 * (abs(rawValue) / abs(setValue));
    if (percent > i*2.5) {
       GD.ColorA(255);
    } else {
       GD.ColorA(60);
    }
    if (i>34) {
       GD.ColorRGB(0xff0000);
    } else if (i>29) {
       GD.ColorRGB(COLOR_ORANGE);
    }else {
       GD.ColorRGB(0x00cc00);
    }
    int position_x1 = x+20 + i*14;
    int position_x2 = position_x1 + 11;
    if (position_x1 > 0 && position_x2 < 800) {
      GD.Vertex2ii(position_x1 ,y);
      GD.Vertex2ii(x+20 + (i+1)*14 - 3, y+9);
    }
    
  }
}


void WidgetsClass::drawBall(int x, int y, bool set) {
  GD.Begin(POINTS);
  GD.PointSize(16 * 6);  
  if (set == true) {
    GD.ColorRGB(255,255,255); 
  } else {
    GD.ColorRGB(0,0,0); 
  }
  GD.Vertex2ii(x, y);
}

void WidgetsClass::measureVoltagePanel(int x, int y, boolean compliance, float limitValueMicro, bool reduceDetails) {
  if (x >= 800) {
    return;
  }
  y=y+28;

  VOLT_DISPLAY.renderMeasured(x /*+ 17*/, y, V_FILTERS.mean, compliance, reduceDetails);
  VOLT_DISPLAY.renderSet(x+120, y+105, limitValueMicro);

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_LIM_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(0); 
  
}

void WidgetsClass::measureCurrentPanel(int x, int y, boolean compliance, bool showBar, CURRENT_RANGE currentRange, float limitValueMicro, bool reduceDetails) {
  if (x >= 800) {
    return;
  }
  
  y=y+28;
  if (currentRange == AMP1 && abs(C_STATS.rawValue) > SETTINGS.max_current_1A_range()) {
    if (showBar) {
      y=y+12; // dont show bar when overflow... just add extra space so the panel gets same size as without overflow...
    }
    GD.ColorA(255);
    CURRENT_DISPLAY.renderOverflowSW(x + 17, y);
  } else 
   if ( (currentRange == MILLIAMP10 && abs(C_STATS.rawValue) > SETTINGS.max_current_10mA_range())) {
    if (showBar) {
      y=y+12; // dont show bar when overflow... just add extra space so the panel gets same size as without overflow...
    }
    GD.ColorA(255);
    CURRENT_DISPLAY.renderOverflowSW(x + 17, y);
  }
  
  else {
    if (showBar) {
      renderBar(x,y, C_STATS.rawValue, limitValueMicro, reduceDetails);
      y=y+12;
    }
    GD.ColorA(255);
    bool shownA = (currentRange == MILLIAMP10);
    shownA = false; // Override. Dont show nA
    GD.ColorA(255);
    CURRENT_DISPLAY.renderMeasured(x, y, C_FILTERS.mean, compliance, shownA, currentRange, reduceDetails); 
  }
  CURRENT_DISPLAY.renderSet(x+120, y+105, limitValueMicro, currentRange);

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_LIM_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(BUTTON_CUR_AUTO);
    if (settingsCurrentAutoRange < 10000) { // auto range disabled
     GD.cmd_button(x+370,y,95,50,29,0,currentRange==AMP1 ? "1A" : "10mA");
  } else {
     GD.cmd_button(x+370,y,95,50,28,0,currentRange==AMP1 ? "1A(A)" : "10mA(A)");
  }
  GD.Tag(0); 
}

void WidgetsClass::measureResistancePanel(int x, int y, boolean compliance, CURRENT_RANGE currentRange, float limitValueMicro) {
  if (x >= 800) {
    return;
  }
  y=y+28;

  VOLT_DISPLAY.renderMeasuredResistance(x /*+ 17*/, y, V_FILTERS.mean, C_FILTERS.mean, compliance);
  //CURRENT_DISPLAY.renderSet(x+120, y+105, SMU1->getLimitValue_micro());
      GD.ColorRGB(COLOR_CURRENT);

  CURRENT_DISPLAY.renderSet(x + 120, y + 105, limitValueMicro, currentRange);

  y=y+105;
  
  GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_LIM_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(0); 

   GD.ColorRGB(0,0,0);
  GD.cmd_fgcolor(0xaaaa90);  
  GD.Tag(BUTTON_LIM_SET);
  GD.cmd_button(x+20,y,95,50,29,0,"LIM");
  GD.Tag(BUTTON_CUR_AUTO);
    if (settingsCurrentAutoRange < 10000) { // auto range disabled
     GD.cmd_button(x+370,y,95,50,29,0,currentRange==AMP1 ? "1A" : "10mA");
  } else {
     GD.cmd_button(x+370,y,95,50,28,0,currentRange==AMP1 ? "1A(A)" : "10mA(A)");
  }
  //GD.cmd_button(x+370,y,95,50,29,0,SMU1->getCurrentRange()==AMP1 ? "1A" : "10mA");
  GD.Tag(0); 
  
  
}

void WidgetsClass::renderVoltageGraph(int x,int y, bool scrolling) {
  V_STATS.renderTrend(x, y, scrolling);
}
void WidgetsClass::renderCurrentGraph(int x,int y, bool scrolling) {
  C_STATS.renderTrend(x, y, scrolling);
}

void WidgetsClass::renderHistogram(int x,int y, bool scrolling) {
  V_STATS.renderHistogram(x,y,scrolling);
}

void WidgetsClass::handleSlidersAndClearUncalRelButtons(int x, int y, bool lessDetails, CURRENT_RANGE currentRange, bool anyDialogOpen) { 
  y=y+40;
  if (!lessDetails) {
    GD.ColorRGB(trackingOngoing==TAG_FILTER_SLIDER?0x00ff00:0xaaaaaa);
    GD.Tag(TAG_FILTER_SLIDER);
    GD.cmd_slider(500+x, y+30, 280,15, OPT_FLAT, V_FILTERS.filterSize * (65535/maxFilterSliderValue), 65535);
    GD.cmd_track(500+x, y+30, 280, 20, TAG_FILTER_SLIDER);
    GD.Tag(0);
  
    GD.Tag(TAG_FILTER_SLIDER_B);
    GD.ColorRGB(trackingOngoing==TAG_FILTER_SLIDER_B?0x00ff00:0xaaaaaa);
    GD.cmd_slider(500+x, y+90, 280,15, OPT_FLAT, V_STATS.getNrOfSamplesBeforeStore()* (65535/maxSamplesSliderValue), 65535);
    GD.cmd_track(500+x, y+90, 280, 20, TAG_FILTER_SLIDER_B);
    GD.Tag(0);  

    GD.ColorRGB(trackingOngoing==TAG_FILTER_SLIDER?0x00ff00:0xaaaaaa);
    GD.cmd_text(500+x, y, 27, 0, "Filter size:");
    GD.cmd_number(580+x, y, 27, 0, V_FILTERS.filterSize);

    GD.ColorRGB(trackingOngoing==TAG_FILTER_SLIDER_B?0x00ff00:0xaaaaaa);
    GD.cmd_text(500+x, y+60, 27, 0, "Samples size:");
    GD.cmd_number(605+x, y+60, 27, 0, V_STATS.getNrOfSamplesBeforeStore());
  }
  
  if (!anyDialogOpen) {
    GD.Tag(BUTTON_CLEAR_BUFFER);
    GD.ColorRGB(DIGIT_UTIL.indicateColor(0x000000, 0x00ff00, 50, BUTTON_CLEAR_BUFFER));
    GD.cmd_button(x+500, y+130, 95, 50, 29, 0, "CLEAR");
    GD.Tag(0);
  }
  
  if (!anyDialogOpen) {
    if (V_CALIBRATION.relativeValueIsSet(currentRange)) {
      GD.ColorRGB(0x00ff00);
    } else {
      GD.ColorRGB(0x000000);
    }
  } else {
    //GD.ColorA(100);
  }
  GD.Tag(BUTTON_REL);
  GD.cmd_button(x+700,y+130,95,50,29,0,"REL");
  GD.Tag(0);

  if (!anyDialogOpen) {
    if (V_CALIBRATION.useCalibratedValues == false) {
      GD.ColorRGB(0x00ff00);
    } else {
      GD.ColorRGB(0x000000);
    }
  }
  
  GD.Tag(BUTTON_UNCAL);
  GD.cmd_button(x+600,y+130,95,50,29,0,"UNCAL");
  GD.Tag(0); // hack to avoid UNCAL button to be called when you press SET on voltage. (when the experimental widget is shown)... 
             // Dont fully understand why this is needed...
  
  GD.ColorA(255);

  if (lessDetails) {
    return;
  }
  GD.get_inputs();
  switch (GD.inputs.track_tag & 0xff) {
    case TAG_FILTER_SLIDER: {
      DEBUG.print("Set filter value:");
      int slider_val = maxFilterSliderValue * GD.inputs.track_val / 65535.0;
      DEBUG.println(slider_val);
      V_FILTERS.setFilterSize(int(slider_val));
      // currently set same as for voltage
      C_FILTERS.setFilterSize(int(slider_val));
      break;
    }
    case TAG_FILTER_SLIDER_B:{
      DEBUG.print("Set samples value:");
      int slider_val = maxSamplesSliderValue * GD.inputs.track_val / 65535.0;
      DEBUG.println(slider_val);
      V_STATS.setNrOfSamplesBeforeStore(int(slider_val));
      // for now, just use same is current as for voltage
      C_STATS.setNrOfSamplesBeforeStore(int(slider_val));
      break;
    }
    default:
      break;
  }
}



void WidgetsClass::renderExperimental(int x, int y, float valM, float setM, bool cur, bool lessDetails, CURRENT_RANGE currentRange, bool anyDialogOpen) {

  handleSlidersAndClearUncalRelButtons(x,y, lessDetails, currentRange, anyDialogOpen);  

  y=y+65;
  x=x+100;
  
  if (cur) {
     //Special handling: set current must currently be positive even if sink/negative.
     //                  This give error when comparing negative measured and positive set.
     //                  Use absolute values to give "correct" comparision...
     setM = abs(setM);
     valM = abs(valM);
  }

  
  float deviationInPercent = 100.0 * ((setM - valM) / setM);
  if (setM == 0.0) {
    deviationInPercent = 100;
  }
  float degrees = -deviationInPercent * 700.0;
  ANALOG_GAUGE.renderAnalogGaugePercent(x+90,y,240, degrees, deviationInPercent, "Deviation from SET", lessDetails);
  
  GD.ColorRGB(0x000000);

  // Visible number on button is half, because the sampling is effectlively the halv because
  // current and voltage is not sampled simultanously in the AD converter !

  int sr = SMU1->getSamplingRate();

  GD.ColorRGB(sr == 5 ? 0x00ff00 : 0x0000);
  GD.Tag(BUTTON_SAMPLE_RATE_5);
  GD.cmd_button(x-50 ,y-20, 95, 40, 29, 0, "2.5Hz");
  
  GD.ColorRGB(sr == 20 ? 0x00ff00 : 0x0000);
  GD.Tag(BUTTON_SAMPLE_RATE_20);
  GD.cmd_button(x-50, y+25, 95, 40, 29, 0, "10Hz");
  
  GD.ColorRGB(sr == 50?0x00ff00 : 0x0000);
  GD.Tag(BUTTON_SAMPLE_RATE_50);
  GD.cmd_button(x-50,y+70,95,40,29,0, "25Hz");

  GD.ColorRGB(sr == 100 ? 0x00ff00 : 0x0000);
  GD.Tag(BUTTON_SAMPLE_RATE_100);
  GD.cmd_button(x-50, y+115, 95, 40, 29, 0, "50Hz");
  GD.Tag(0);
  
}

void WidgetsClass::widgetBodyHeaderTab(int y, int activeWidget) {
  y=y+10;
  GD.Begin(RECTS);

  // tab light shaddow
  GD.ColorA(255);
  GD.LineWidth(350);
  GD.ColorRGB(0x555555);
  GD.Vertex2ii(21,y-2);
  GD.Vertex2ii(778, 480);
  
  // tab 
  GD.ColorA(255);
  GD.LineWidth(350);
  GD.ColorRGB(0x2c2c2c);
  GD.Vertex2ii(22,y);
  GD.Vertex2ii(778, 480);

  // Why does this blend to dark gray instead of being just black ?
  // Is it because the rectangle above "shines" though ? It is not set to transparrent...
  // must learn more about the blending....
  GD.Begin(RECTS);
  GD.ColorA(180); // slightly transparrent to grey shine though a bit
  GD.LineWidth(10);
  GD.ColorRGB(0x000000);
  GD.Vertex2ii(0,y+15);
  GD.Vertex2ii(799, 480);

  // strip between tab heading and content
  GD.Begin(LINE_STRIP);
  GD.ColorA(200);
  GD.LineWidth(10);
  GD.ColorRGB(COLOR_CURRENT_TEXT);
  GD.Vertex2ii(0,y+15+2);
  GD.Vertex2ii(798, y+15+2);
  GD.ColorA(255);


  y=y-2;
  int x = 400 - 30 * noOfWidgets/2;
  if (MAINMENU.active != true) {
    for (int i = 0; i < noOfWidgets; i++) {
      drawBall(x+ i*30,y,activeWidget == i);
    }
  }
  
}

void WidgetsClass::handleWidgetScrollPosition(int gestureDetected) {
  if (gestureDetected == GEST_MOVE_LEFT) {
    scrollDir = -1;
  } 
  else if (gestureDetected == GEST_MOVE_RIGHT) {
    scrollDir = 1;
  } 
  
  scroll = scroll + scrollDir * scrollSpeed;
  if (scroll <= -800 && scrollDir != 0) {
    activeWidget ++;
    if (activeWidget > noOfWidgets -1) {
      // swap arund
      activeWidget = 0;
    }
    scrollDir = 0;
    scroll = 0;
  } else if (scroll >= 800 && scrollDir != 0) {
    activeWidget --;
    if (activeWidget < 0) {
      // swap around
      activeWidget = noOfWidgets - 1;
    }
    scrollDir = 0;
    scroll = 0;
  }
}

void WidgetsClass::displayWidget( FUNCTION_TYPE functionType, OPERATION_TYPE operationType, bool hasCompliance, bool reduceDetails, float getSetValue_micro, float limitValue_micro, CURRENT_RANGE currentRange, bool anyDialogOpen, int yPosLowerPart) {
   widgetBodyHeaderTab(yPosLowerPart, activeWidget);

  if (activeWidget >= 0) {
    if (scrollDir == 0) {
      showWidget(yPosLowerPart, activeWidget, 0    ,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro,currentRange, anyDialogOpen);
    }
    else if (scrollDir == -1) {
      showWidget(yPosLowerPart,activeWidget, scroll,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro, currentRange, anyDialogOpen);
      if (activeWidget == noOfWidgets - 1) {
        // swap from last to first
        showWidget(yPosLowerPart, 0, scroll + 800,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro,currentRange, anyDialogOpen);
      } else {
        showWidget(yPosLowerPart,activeWidget + 1, scroll + 800,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro,currentRange, anyDialogOpen);
      }
    } 
    else if (scrollDir == 1) {
      if (activeWidget == 0) { 
        // swap from first to last
        showWidget(yPosLowerPart,noOfWidgets -1 , scroll - 800,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro,currentRange, anyDialogOpen);
      } else {
        showWidget(yPosLowerPart,activeWidget - 1, scroll - 800,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro,currentRange, anyDialogOpen);
      }
      showWidget(yPosLowerPart,activeWidget, scroll + 0,functionType, operationType, hasCompliance, reduceDetails, getSetValue_micro,limitValue_micro,currentRange, anyDialogOpen);
    }   
  }
  
}

void WidgetsClass::checkButtons(int tag) {
  if (tag == BUTTON_SAMPLE_RATE_5 or tag == BUTTON_SAMPLE_RATE_20 or tag == BUTTON_SAMPLE_RATE_50 or tag == BUTTON_SAMPLE_RATE_100) { //TODO: Change name
      if (timeSinceLastChange + 500 < millis()){
        timeSinceLastChange = millis();
      } 
     
      if (tag == BUTTON_SAMPLE_RATE_5) {
              DEBUG.println("BUTTON_SAMPLE_RATE_5");

        SMU1->setSamplingRate(5); 
      }
      if (tag == BUTTON_SAMPLE_RATE_20) {
                      DEBUG.println("BUTTON_SAMPLE_RATE_20");

        SMU1->setSamplingRate(20);
      }
      if (tag == BUTTON_SAMPLE_RATE_50) {
                      DEBUG.println("BUTTON_SAMPLE_RATE_50");
        SMU1->setSamplingRate(50);
      }
       if (tag == BUTTON_SAMPLE_RATE_100) {
                              DEBUG.println("BUTTON_SAMPLE_RATE_100");

        SMU1->setSamplingRate(100);
      }

  } else if (tag == BUTTON_REL) {
      DEBUG.println("Set relative");
      V_CALIBRATION.toggleRelativeValue(V_STATS.rawValue, SMU1->getCurrentRange());
      C_CALIBRATION.toggleRelativeValue(C_STATS.rawValue, SMU1->getCurrentRange());
    } else if (tag == BUTTON_UNCAL) {
      DEBUG.println("Uncal set");
      V_CALIBRATION.toggleCalibratedValues();
      C_CALIBRATION.toggleCalibratedValues();
    } else if (tag == BUTTON_CLEAR_BUFFER) {
      DEBUG.println("clearbuffer set");

      SIMPLE_STATS.clear(); // TODO: Separate clearing for this ?
      V_STATS.clearBuffer();
      C_STATS.clearBuffer();
      DIGIT_UTIL.startIndicator(tag); 
    } 
}


WidgetsClass WIDGETS;

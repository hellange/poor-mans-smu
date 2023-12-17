
#ifndef WIDGETS_H
#define WIDGETS_H


#include <stdint.h>
#include "Filters.h"
#include "Mainmenu.h"
#include "Calibration.h"
#include "Stats.h"
#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "colors.h"
#include "current_display.h"
#include "volt_display.h"
#include "analogGauge.h"
#include "debug.h"
#include "digit_util.h"
#include "Settings.h"
#include "Gest.h"
#include "SimpleStats.h"
#include "SMU_HAL.h"

class WidgetsClass {
public:
  void init(SMU_HAL &SMU);
  void displayWidget( FUNCTION_TYPE functionType, OPERATION_TYPE operationType, bool hasCompliance, bool reduceDetails, float getSetValue_micro , float limitValue_micro, CURRENT_RANGE currentRange, bool anyDialogOpen, int yPosLowerPart);
  void showWidget(int y, int widgetNo, int scroll, FUNCTION_TYPE functionType, OPERATION_TYPE operationType, bool hasCompliance, bool reduceDetails, float getSetValue_micro,float limitValue_micro, CURRENT_RANGE currentRange, bool anyDialogOpen);
  void handleWidgetScrollPosition(int gestureDetected);
  bool isScrolling();
  void checkButtons(int tag);

  int settingsCurrentAutoRange = 0; // TODO: Currently indicate position of toggle and is being used by main program.
                                    //       The toggle value itself should NOT be used as a setting...
                                    //       So it should be private and main program should use something else...
  
  // Related to sliders. Action here is accounted for when i.e. checking swipe etc...
  // TODO: "collision" between toggle manipulating and gestures should be solved in a better way...
  int sliderTags[2] = {TAG_FILTER_SLIDER, TAG_FILTER_SLIDER_B};
  int trackingOngoing = 0; // Indicates if any toggle/slider is being used. To avoid scroll gesture when manipulating toggle...
                           // TODO: Should be solved differently

private:
  SMU_HAL *SMU1;

  int maxFilterSliderValue = 50;
  int maxSamplesSliderValue = 100;

  // Used by scrolling related to widgets (lower part of the main screen)
  int scrollSpeed = 70;
  int scroll = 0;
  int noOfWidgets = 7;
  int activeWidget = 0;
  int scrollDir = 0;

  void measureVoltagePanel(int x, int y, bool compliance, float limitValueMicro, bool reduceDetails);
  void measureCurrentPanel(int x, int y, bool compliance, bool showBar,  CURRENT_RANGE currentRange, float limitValueMicro, bool reduceDetails);
  void measureResistancePanel(int x, int y, bool compliance,CURRENT_RANGE currentRange, float limitValueMicro);
  void renderVoltageGraph(int x,int y, bool scrolling);
  void renderCurrentGraph(int x,int y, bool scrolling);
  void renderHistogram(int x,int y, bool scrolling);
  void handleSlidersAndClearUncalRelButtons(int x, int y, bool lessDetails,CURRENT_RANGE currentRange, bool anyDialogOpen);
  void renderExperimental(int x, int y, float valM, float setM, bool cur, bool lessDetails, CURRENT_RANGE currentRange, bool anyDialogOpen);
  void widgetBodyHeaderTab(int y, int activeWidget);
  void renderBar(int x, int y, float rawValue, uint64_t setValue_u, bool reduceDetails);
  void drawBall(int x, int y, bool set);

  uint32_t timeSinceLastChange = 0;
};

extern WidgetsClass WIDGETS;
#endif
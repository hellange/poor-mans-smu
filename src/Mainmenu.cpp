
#include "Mainmenu.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"
#include "FunctionPulse.h"
#include "Debug.h"

MainMenuClass MAINMENU;

void MainMenuClass::open(void (*closedMenuFn)(FUNCTION_TYPE type)) {
  active = true;
  closedMainMenuFn = closedMenuFn;
  scrollMainMenuDir = 1;
}

void MainMenuClass::close() {
    
    scrollMainMenuDir = -1; 

    //TODO: Find a better way to make sure the closeMainMenuFn is called only once...
    if (active) {
      DEBUG.println("MainMenuClass::close");
      closedMainMenuFn(functionType);
    }
    
}

void MainMenuClass::handleButtonAction(int inputTag) {
    if (scrollMainMenuDir != 0) {
      return; // dont try to detec keypress while main is animating up or down.
    }
    if (inputTag == MENU_BUTTON_SOURCE_PULSE) {
      functionType = SOURCE_PULSE;
      close();
    } else if (inputTag == MENU_BUTTON_SOURCE_DC_VOLTAGE) {
      functionType = SOURCE_DC_VOLTAGE;
      close();
    } else if (inputTag == MENU_BUTTON_SOURCE_DC_CURRENT) {
      functionType = SOURCE_DC_CURRENT;
      close();
    }else if (inputTag == MENU_BUTTON_SOURCE_SWEEP) {
      functionType = SOURCE_SWEEP;
      close();
    } else if (inputTag == MENU_BUTTON_DIGITIZE) {
      functionType = DIGITIZE;
      close();
    } else if (inputTag == MENU_BUTTON_GRAPH) {
      functionType = GRAPH;
      close();
    } else if (inputTag == MENU_BUTTON_RESISTANCE) {
      functionType = MEASURE_RESISTANCE;
      close();
    } else if (inputTag == MENU_BUTTON_LOGGER) {
      functionType = DATALOGGER;
      close();
    }
}

void MainMenuClass::render() {
    int scrollSpeed = 30;
    scrollMainMenu = scrollMainMenu + scrollMainMenuDir*scrollSpeed;
    if (scrollMainMenu > 350) {
      scrollMainMenu = 350;
      scrollMainMenuDir = 0;
    }
if (toBeDisactive != true) {
    GD.ColorA(230);   

    GD.Begin(RECTS);
    GD.LineWidth(200);
    GD.ColorRGB(0x888888);
    GD.Vertex2ii(50,0);
    GD.Vertex2ii(750, scrollMainMenu+40);

    GD.Begin(RECTS);
    GD.LineWidth(180);
    //GD.ColorA(255);
    GD.ColorRGB(0x000000);
    GD.Vertex2ii(750, scrollMainMenu+40);
    GD.ColorRGB(0xffffff);

    //GD.ColorA(230);

    int buttonWidth = 200;
    int buttonHeight = 55;
               
               
    GD.ColorA(255);   

    const char * text[4][3] = {
    {"SOURCE VOLT\0", "SOURCE CURRENT\0", "LOGGER\0"},
    {"ELECTRONIC LOAD\0", "VOLTMETER\0", "PULSE GENERATOR\0"},
    {"SWEEP\0", "GRAPH\0", "DIGITIZE\0"},
    {"BATTERY SIM\0", "BATTERY MOD\0", "RESISTANCE\0"}
    };
    for (int y =0;y<4;y++) {
      for (int x =0;x<3;x++) {
        bool enable = false;
        if (y==1 && x == 2) {
          GD.Tag(MENU_BUTTON_SOURCE_PULSE);
          enable = true;
        } else if (y==0 && (x==0)) {
          GD.Tag(MENU_BUTTON_SOURCE_DC_VOLTAGE);
          enable = true;

        } else if (y==0 && (x==1)) {
          GD.Tag(MENU_BUTTON_SOURCE_DC_CURRENT);
          enable = true;

        }else if (y==0 && (x==2)) {
          GD.Tag(MENU_BUTTON_LOGGER);
          enable = true;

        } else if (y==2 && x==0) {
          GD.Tag(MENU_BUTTON_SOURCE_SWEEP);
          enable = true;

        } else if (y==2 && x==2) {
          GD.Tag(MENU_BUTTON_DIGITIZE);
          enable = true;

        } else if (y==2 && x==1) {
          GD.Tag(MENU_BUTTON_GRAPH);
          enable = true;
        } else if (y==3 && x==2) {
          GD.Tag(MENU_BUTTON_RESISTANCE);
          enable = true;
        } else {
          GD.Tag(0);
        }
        if (enable) {
              GD.ColorRGB(0x4444ff);
        } else {
              GD.ColorRGB(0x666666);
        }
        GD.cmd_button(70+(buttonWidth+30)*x,scrollMainMenu-320+(buttonHeight+20)*y,buttonWidth,buttonHeight,28,0,text[y][x]);
      }
    }
               
    GD.Tag(MAIN_MENU_CLOSE);
    GD.cmd_button(360,scrollMainMenu-10,80,40,28,0,"CLOSE");
}
    if(GD.inputs.tag == MAIN_MENU_CLOSE && scrollMainMenuDir == 0) {
      close();
    }
  
  if (scrollMainMenuDir == -1){
      scrollMainMenu = scrollMainMenu + scrollMainMenuDir*15;
      if (scrollMainMenu < 0) {
        keepActiveForAWhileTimer = millis();
        scrollMainMenu = 0;
        //active = false;
        toBeDisactive = true;
        scrollMainMenuDir = 0;
      }
  }
  if (toBeDisactive == true && keepActiveForAWhileTimer + 100 < millis()) {
    active = false;
    toBeDisactive = false;
  }
}

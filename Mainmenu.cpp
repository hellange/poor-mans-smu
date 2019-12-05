
#include "Mainmenu.h"

#include <SPI.h>
#include "GD2.h"
#include "tags.h"

MainMenuClass MAINMENU;

void MainMenuClass::activate() {
  active = true;
  scrollMainMenuDir = 1;
}
void MainMenuClass::handle() {

    scrollMainMenu = scrollMainMenu + scrollMainMenuDir*25;
    if (scrollMainMenu > 350) {
      scrollMainMenu = 350;
      scrollMainMenuDir = 0;
    }

    GD.Begin(RECTS);
    GD.LineWidth(200);
    //GD.ColorA(255);   // allready opaque, why ?
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
    int buttonHeight = 70;
               
    const char * t[3][3] = {
    {"SOURCE VOLT\0", "SOURCE CURRENT\0", "BATTERY\0"},
    {"ELECTRONIC LOAD\0", "VOLTMETER\0", "PULSE GENERATOR\0"},
    {"SWEEP\0", "RESISTANCE\0", "SETTINGS\0"}};
    GD.ColorRGB(0x444444);
    for (int y =0;y<3;y++) {
      for (int x =0;x<3;x++) {
        GD.cmd_button(70+(buttonWidth+30)*x,scrollMainMenu-280+(buttonHeight+20)*y,buttonWidth,buttonHeight,28,0,t[y][x]);
      }
    }
    GD.Tag(MAIN_MENU_CLOSE);
    GD.cmd_button(360,scrollMainMenu-10,80,40,28,0,"CLOSE");

    if(GD.inputs.tag == MAIN_MENU_CLOSE && scrollMainMenuDir == 0) {
      scrollMainMenuDir = -1;
    }
  
  if (scrollMainMenuDir == -1){
      scrollMainMenu = scrollMainMenu + scrollMainMenuDir*15;
      if (scrollMainMenu < 0) {
        scrollMainMenu = 0;
        active = false;
        scrollMainMenuDir = 0;
      }
  }
}

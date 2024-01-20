#ifndef UIBUTTON_H
#define UIBUTTON_H

#include "Arduino.h"
#include <SPI.h>
#include "GD2.h"

#include <functional>

class UIButtonClass {

  //enum class PressType{ InitialPress, ShortPressAfterRelease, LongPress, LongPressAfterRelease};

  int BUTTON_DEFAULT_COLOR = 0xaaaa90;
  int BUTTON_PRESSED_COLOR = 0x889088;
  int BUTTON_PRESSED_COLOR_ALT = 0x669066; // blink or identify
  int BUTTON_TEXT_DEFAULT_COLOR = 0x000000;
  int BUTTON_TEXT_PRESSED_COLOR = 0x00ff00;
  static int current_pressed_color; // This is dynamic in order to blink while holding button...


  float PRSM = 0.90; // button pressed size adjustment

  std::function<void(bool)> handlerFn;
private:
  bool singlePressMode = false; // Set to true if holding button shall give only one function call. With boolean indicating if long or short click.
                                // Set to false of multiple velocity adjusted function calls while holding button

  int id;
  const char *text;
  int x;
  int y;
  int width;
  int height;
  void checkPressType(int id);
  void preCallbackFn(bool longPress);
  void render();
  void drawButton(int x, int y, int width, int height, const char* text_, bool pressed, bool initialTouch);

  void handle();

  int minimumPressTime = 0; //30; // Adjust so that it feels right. 50 might be on the slow side, 
                             // but prevents clicking button whils sliding finger quickly accross screen...

public:

  static int noOfButtons;

  static bool pressed;
  static int pressedId;
  static bool released;
  static bool longPress;
  static bool needReleaseFirst;

  static uint32_t pressedAt;
  static uint32_t debouceTimer;
  static uint32_t debouceTimeoutButton;

  void init(int id, int x, int y, int width, int height, const char* text, std::function<void(bool)> handlerFn_, bool singleClick);
};

#endif
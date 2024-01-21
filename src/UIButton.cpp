#include "UIButton.h"
#include "Debug.h"

// Ended up with a bunch of static variables for button timing etc.
// Could avoid that if button objects were not created every time each graphics rendering loop...
// TODO: Revisit this concept...
int UIButtonClass::noOfButtons = 0;
bool UIButtonClass::pressed = false;
int UIButtonClass::pressedId = 999;
bool UIButtonClass::released = false;
bool UIButtonClass::hasBeenPressedLong = false;
bool UIButtonClass::needReleaseFirst = false;
int UIButtonClass::current_pressed_color = 0x0000ff; //0x888890; //BUTTON_PRESSED_COLOR;

uint32_t UIButtonClass::statusTimerXX = millis();

uint32_t UIButtonClass::pressedAt = 0;
uint32_t UIButtonClass::debouceTimer = 0;
uint32_t UIButtonClass::debouceTimeoutButton = 0;
//function<int(int, int)> func
//void UIButtonClass::init(int id_, int x_, int y_, int width_, int height_, const char* text_, auto (*handlerFn_)(bool longPress), bool singleClick) {

void UIButtonClass::init(int id_, int x_, int y_, int width_, int height_, const char* text_, std::function<bool(PressType)> handlerFn_, bool singleClick) {
   handlerFn = handlerFn_;
   singlePressMode = singleClick;
   id = id_;
   noOfButtons++;
   text = text_;
   x = x_;
   y = y_;
   width = width_;
   height = height_;
   render();
   handle();
}

void UIButtonClass::render() {
  GD.Tag(id);
  if (id == pressedId && pressed) {
      if (pressedAt + minimumPressTime > millis()) {
        // initial touch. Is ised to indicate that its a touch, but maybe not long enough to be detected as a button click
        drawButton(x, y , width, height, text, true, true);
      } else {
        drawButton(x, y, width, height, text, true, false);
      }
  } else {
      drawButton(x, y, width, height, text, false, false);
  }
  
  GD.cmd_fgcolor(BUTTON_DEFAULT_COLOR); 

}

// Override you you want other look & feel
// void UIButtonClass::drawButton(int x, int y, int width, int height, const char* text_, bool pressed, bool initialTouch) {
//   if (pressed && initialTouch) {
//         // initial touch. Is ised to indicate that its a touch, but maybe not long enough to be detected as a button click
//         GD.cmd_fgcolor(BUTTON_DEFAULT_COLOR);
//         GD.ColorRGB(BUTTON_TEXT_PRESSED_COLOR);
//         //GD.cmd_fgcolor(BUTTON_INITALTOUCH_COLOR); // color initially when touched
//         GD.cmd_button(x, y, width, height, 29, OPT_NOTEAR, text);
//   } else if (pressed) {
//         GD.cmd_fgcolor(current_pressed_color); // color when probably a button press (not too fast)
//         GD.ColorRGB(BUTTON_TEXT_PRESSED_COLOR);
//         GD.cmd_button(x + width*(1-PRSM) / 2, y + height*(1-PRSM) / 2, width*PRSM, height*PRSM, 29, OPT_NOTEAR, text);
//   } else {
//       GD.cmd_fgcolor(BUTTON_DEFAULT_COLOR); 
//       GD.ColorRGB(0,0,0);
//       GD.cmd_button(x, y, width, height, 29, OPT_NOTEAR, text);
//   }
// }
void UIButtonClass::drawButton(int x, int y, int width, int height, const char* text_, bool pressed, bool initialTouch) {
// button with square size. origin in the middle of button !
int sz = width/2 + height * 0;

GD.Begin(RECTS);
GD.ColorA(200);
GD.ColorRGB(200,200,200);
GD.LineWidth(16 * 20);
GD.Vertex2ii(x - sz, y - sz);
GD.Vertex2ii(x + sz, y + sz);

GD.ColorA(200);
GD.ColorRGB(255,255,255);
GD.LineWidth(16 * 15);
GD.Vertex2ii(x - sz, y - sz);
GD.Vertex2ii(x + sz, y + sz);
GD.ColorA(0xff);
GD.ColorRGB(0,0,0);
int fontsize = 29;
GD.cmd_text(x, y, fontsize, OPT_CENTER, text);
GD.ColorA(255);

}


void UIButtonClass::handle() {
  int tag = GD.inputs.tag;

  if (statusTimerXX + 1000 < millis()) {
            //     DEBUG.print("status");
            //     DEBUG.print("pressed=");
            // DEBUG.print(pressed);
            // DEBUG.print(", pressedId=");
            // DEBUG.print(pressedId);
            // DEBUG.print(",id=");
            // DEBUG.print(id);
            // DEBUG.print(",tag=");
            // DEBUG.println(tag);
statusTimerXX = millis();
  }


  if (tag == id) {
            //DEBUG.print("xxxxxx tag == id, id=");
            //DEBUG.println(id);
           // relasedByOtherThanButtonRelease = false;
    checkPressType(id);
  } else if (tag == 0 && pressedId == id /*&& !relasedByOtherThanButtonRelease */) {

    // Button is released.
    // For operations that can be determined only after button has been released...

    if (pressed && pressedAt + minimumPressTime > millis()) {
        // dont treat very quick touches as button press.
        // TODO: Avoid detecting buttons when swiping the display.
        //       Make check that previous coordinates are not to different ???
        // For now, just ignore this button press, by clearing 
        DEBUG.println("Ignore quick button touch...");
        released = true;
        pressed = false;
        debouceTimeoutButton = 5000;
        pressedAt = 0;
        debouceTimer = 0;
    }
    needReleaseFirst = false;
    if (pressed && singlePressMode) {
        // It was pressed before release
        released = true;
        pressed = false;
        // This means that button has been released after press, so now we can do operation
        if (hasBeenPressedLong) {
            preCallbackFn(PressType::LongPressAfterRelease);
        } else {
            //pressed = false;
            preCallbackFn(PressType::ShortPressAfterRelease);
        }
        hasBeenPressedLong = false;
    } else if (pressed && !singlePressMode) {
        released = true;
        pressed = false;
        debouceTimeoutButton = 5000;
    }
    pressedAt = 0;
    debouceTimer = 0;
  }
}

void UIButtonClass::checkPressType(int id) {
    if (!pressed) {
        current_pressed_color = BUTTON_PRESSED_COLOR;
        bool doResetOfButtons = false;
        if (singlePressMode) {
            DEBUG.println("PRESSING UI BUTTON INITIALLY (SINGLE PRESS MODE)");
            debouceTimeoutButton = 50;
            doResetOfButtons = preCallbackFn(PressType::InitialPress);
        } else {
            DEBUG.println("PRESSING UI BUTTON INITIALLY");
            debouceTimeoutButton = 250; // Initial debounce before decreasing
            preCallbackFn(PressType::InitialPress);
            doResetOfButtons = false; // not relevant when not in single press mode ?
        }

        // reset if button state is important if the button callback
        // results in buttons disappear before they are released.
        // For example if button press causes a completely new UI.
        if (doResetOfButtons) {
          pressed = false;
          pressedId = 0;
          pressedAt = 0;
          released = true;
        } else  {
          hasBeenPressedLong = false;
          pressed = true;
          pressedId = id;
          released = false;
          pressedAt = millis();
          debouceTimer = millis();
        } 
        
    } else if (pressed && debouceTimer + debouceTimeoutButton < millis() /* debouce time*/) {
        debouceTimer = millis();
        if (!singlePressMode) {
          // blinking...
          //TODO When blinking too fast, it will not show... How to fix that ?
          if (current_pressed_color == BUTTON_PRESSED_COLOR) {
            current_pressed_color = BUTTON_PRESSED_COLOR_ALT;
          } else {
            current_pressed_color = BUTTON_PRESSED_COLOR;
          }

          preCallbackFn(PressType::ShortPressHolding);
          // Determine how fast button function shall be called depending on how long its held
          // Faster and faster the longer the button is pressed...
          if (pressedAt + 4000 < millis()) {
            debouceTimeoutButton = 10;
          } else if (pressedAt + 3000 < millis()) {
            debouceTimeoutButton = 50;
          } else if (pressedAt + 2000 < millis()) {
            debouceTimeoutButton = 100;
          } else if (pressedAt + 1000 < millis()) {
            debouceTimeoutButton = 150;
          }  
        }
        else if (pressedAt + 1000 < millis() && needReleaseFirst == false) {
            hasBeenPressedLong = true;
            //DEBUG.println(id);
            //DEBUG.println("!!! DO LONGPRESS OPERATION DURING KEY PRESS !!!!");
            preCallbackFn(PressType::LongPressHolding);
            needReleaseFirst = true; // to prevent another long press before being released first!
        }
    } 

}

void UIButtonClass::pressTypePrint(UIButtonClass::PressType pressType) {
   if (pressType == PressType::LongPressHolding) {
        DEBUG.print("BUTTON LongPress");
    } else if (pressType == PressType::InitialPress) {
        DEBUG.print("BUTTON InitialPress");
    } else if (pressType == PressType::LongPressAfterRelease) {
        DEBUG.print("BUTTON LongPressAfterRelease");
    } else if (pressType == PressType::ShortPressAfterRelease) {
        DEBUG.print("BUTTON ShortPressAfterRelease");
    } else if (pressType == PressType::ShortPressHolding) {
        DEBUG.print("BUTTON ShortPressHolding");
    } else {
        DEBUG.print("BUTTON unknown pressType");
    }
}

// The callback function parameter contains what type of button press that was detected.
//
// Important:
//   For single press mode, the callback should return true if
//   the callback function results in UI changes that rearrange, add or remove buttons !
//   Else the state might result in strange behaviour afterwords...
//
//   If the callback does not result in such changes, it should return false. Else it could
//   result in multiple callbacks because the states have been reset...

bool UIButtonClass::preCallbackFn(PressType pressType) {
    UIButtonClass::pressTypePrint(pressType);
    DEBUG.print(", id:");
    DEBUG.println(id, DEC);
    // Actual function to call
    bool done = handlerFn(pressType);
    return done;
}

#include "UIButton.h"
#include "Debug.h"

// Ended up with a bunch of static variables for button timing etc.
// Could avoid that if button objects were not created every time each graphics rendering loop...
// TODO: Revisit this concept...
int UIButtonClass::noOfButtons = 0;
bool UIButtonClass::pressed = false;
int UIButtonClass::pressedId = 999;
bool UIButtonClass::released = false;
bool UIButtonClass::longPress = false;
bool UIButtonClass::needReleaseFirst = false;
int UIButtonClass::current_pressed_color = 0x0000ff; //0x888890; //BUTTON_PRESSED_COLOR;

uint32_t UIButtonClass::pressedAt = 0;
uint32_t UIButtonClass::debouceTimer = 0;
uint32_t UIButtonClass::debouceTimeoutButton = 0;
//function<int(int, int)> func
//void UIButtonClass::init(int id_, int x_, int y_, int width_, int height_, const char* text_, auto (*handlerFn_)(bool longPress), bool singleClick) {

void UIButtonClass::init(int id_, int x_, int y_, int width_, int height_, const char* text_, std::function<void(bool)> handlerFn_, bool singleClick) {
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
  if (tag == id) {
    checkPressType(id);
  } else if (tag == 0 && pressedId == id) {

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
        if (longPress) {
            // Include stuff here that shall be done AFTER a longpress has been released
            //DEBUG.println("!!! DO LONGPRESS OPERATION AFTER RELEASE !!!!");
        } else {
            // Include stuff here that shall be done after normal button press
            //DEBUG.print("!!!! DO PRESS OPERATION !!!! ID=");
            //DEBUG.print(pressedId);
            //DEBUG.print(",id=");
            //DEBUG.println(id);
            preCallbackFn(false);
        }
        longPress = false;
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

    // Button was detected as newly pressed
    // Now start debouce and singlePress or long press handling setup
    if (!pressed) {
        //DEBUG.print("PRESSED UI BUTTON FIRST");
        //DEBUG.print(id);
        current_pressed_color = BUTTON_PRESSED_COLOR;
        
        if (singlePressMode) {
            DEBUG.print("PRESSING UI BUTTON INITIALLY (SINGLE PRESS MODE)");
            debouceTimeoutButton = 50;

        } else {
            DEBUG.print("PRESSING UI BUTTON INITIALLY (NOT IN SINGLE PRESS MODE. CALL CALLBACK...");
            debouceTimeoutButton = 250; // Initial debounce before decreasing
            preCallbackFn(false); // Just hardcode longPress boolean to false
        }

        longPress = false;
        pressed = true;
        pressedId = id;
        released = false;
        pressedAt = millis();
        debouceTimer = millis();
    } else if (pressed && debouceTimer + debouceTimeoutButton < millis() /* debouce time*/) {
        debouceTimer = millis();
        if (!singlePressMode) {
          //DEBUG.print("PRESSING UI BUTTON AFTER HOLDING IT FOR A WHILE at ");
          //DEBUG.print(millis());
          //DEBUG.print(",Debouce timeout is ");
          //DEBUG.println(debouceTimeoutButton, DEC);

          // blinking...
          //TODO When blinking too fast, it will not show... How to fix that ?
          if (current_pressed_color == BUTTON_PRESSED_COLOR) {
            current_pressed_color = BUTTON_PRESSED_COLOR_ALT;
          } else {
            current_pressed_color = BUTTON_PRESSED_COLOR;
          }

          preCallbackFn(false); // Just hardcode longPress boolean to false
          // Determine how fast button function shall be called depending on how long its held
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
        else if (pressedAt + 1000 < millis()  /* long hold time*/ && needReleaseFirst == false) {
            longPress = true;
            //DEBUG.println(id);
            DEBUG.println("!!! DO LONGPRESS OPERATION DURING KEY PRESS !!!!");
            preCallbackFn(true);
            needReleaseFirst = true; // to prevent another long press before being released first!
        }
    } 

}

void UIButtonClass::preCallbackFn(bool longPress) {
    if (longPress) {
        DEBUG.print("LONG BUTTON CLICK, id=");
        pressed = false; // Clear state. Needed if for example a previous press has caused a new UI page 
                         // so button was never detected as released !
    } else {
        DEBUG.print("SHORT BUTTON CLICK, id=");
    }
    DEBUG.println(id, DEC);

    //DEBUG.print(id);
    // DEBUG.print(", text");
    // DEBUG.print(text);
    // DEBUG.print(", millis:");
    // DEBUG.println(millis());

    // Actual function to call

    //current_pressed_color = BUTTON_PRESSED_COLOR; 
    handlerFn(longPress);
}

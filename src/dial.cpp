#include "dial.h"
#include "math.h"
#include "colors.h"
#include <SPI.h>
#include "GD2.h"
#include "Settings.h"
#include "Debug.h"


DialClass SOURCE_DIAL;
DialClass LIMIT_DIAL;

void DialClass::close() {
    dialog = false;
}

void DialClass::open(int type, int set_or_limit_, void (*callbackFn)(int set_or_limit, bool cancel), int64_t micro) {
  //TODO: Convert value to visible digits. Right now it uses values/digits stored in the DialClass itself !
  //char cbuf[20];
  //sprintf(cbuf, "%10.5f", value); 
  //DEBUG.print("Set string:");
  //DEBUG.println(cbuf);
  //DEBUG.flush();


  closedFn = callbackFn;
  vol_cur_type = type;
  set_or_limit = set_or_limit_;
  dialog=true; 

   DEBUG.print("Opening ");
   DEBUG.println(set_or_limit == LIMIT ? "LIMIT DIAL" : "SOURCE DIAL");
   DEBUG.println(type == SOURCE_VOLTAGE ? "SOURCING VOLTAGE" : "SOURCING CURRENT");
   DEBUG.print("Input value:");
   //DEBUG.print(micro/1000.0,3);
   //DEBUG.println("(milli)");
   DEBUG.println(micro/1.0,1);


 

  setMv(micro/1000.0);
  /*
  // if not set, use some defaults...
  if (digits == 0){
  // These settings should not be hardcoded here
    if (vol_cur_type == SOURCE_VOLTAGE && set_or_limit == SET) {
      digits = 1;
      dialEntries[0] = 1; 
      strncpy(voltDecade, "V" ,2);
    } else if (vol_cur_type == SOURCE_CURRENT && set_or_limit == SET){
      digits = 3;
      dialEntries[0] = 1;  
      dialEntries[1] = 0;  
      dialEntries[2] = 0;  
      strncpy(voltDecade, "mV" ,2);
    } else if (vol_cur_type == SOURCE_VOLTAGE && set_or_limit == LIMIT){
      digits = 3;
      dialEntries[0] = 1;  
      dialEntries[1] = 0;  
      dialEntries[2] = 0;  
      strncpy(voltDecade, "mV" ,2);
    } else if (vol_cur_type == SOURCE_CURRENT && set_or_limit == LIMIT){
      digits = 2;
      dialEntries[0] = 1;
      dialEntries[1] = 0;  
      strncpy(voltDecade, "V" ,2);
    }
  }
  */
}

void DialClass::init() {
  negative = false;
  mv=0.0;
  clear();
}

// TODO: set is also used for current. change name of mv variable ?
void DialClass::setMv(float mv) {
  DEBUG.print("Set:");
  DEBUG.println(mv,3 );
  if (mv < 0.0) {
    negative = true;
  } else {
    negative = false;
  }

  mv = abs(mv);
  
  char outstr[15];
  int nrOfDigitsBeforeComma = 1;
  if (mv>9999) {
    nrOfDigitsBeforeComma = 5;
  }
  else if (mv>999) {
    nrOfDigitsBeforeComma = 4;
  }
  else if (mv>99) {
    nrOfDigitsBeforeComma = 3;
  }
  else if (mv>9) {
    nrOfDigitsBeforeComma = 2;
  }
  int nfOfDigitsAfterComma = 2; // TODO: Adhere to the rules on number of digits "allowed" in various situations.

  dtostrf(mv,nrOfDigitsBeforeComma + nfOfDigitsAfterComma, 4, outstr);
  DEBUG.print("Get individual digit elements to display based on value ");
  DEBUG.println(mv,5);
  int dialEntriesIndex = 0;
  DEBUG.print("Digits: ");
  for (int i=0; i< nrOfDigitsBeforeComma + nfOfDigitsAfterComma + 1 ; i++) {
    if (outstr[i]!=0x20 && outstr[i]!='-') {
      DEBUG.printchar(outstr[i]);
      DEBUG.print(",");
      if (outstr[i] == '.') {
        dialEntries[dialEntriesIndex++] = KEYBOARD_COMMA;
      } else {
        dialEntries[dialEntriesIndex++] = outstr[i] - '0';
      }
    }
   
  }
  digits = dialEntriesIndex;
  DEBUG.print(" # of digit elements: ");
  DEBUG.println(dialEntriesIndex, 10);
  strncpy(voltDecade, "mV" ,2);
}

float DialClass::getMv() {
  if (negative) {
    return -mv;
  } else {
    return mv;
  }
}

bool DialClass::isDialogOpen() {
  return dialog;
}

void DialClass::clear() {
  error = false;
  warning = false;
  digits = 0;
  if (vol_cur_type == SOURCE_VOLTAGE) {
      strncpy(voltDecade, "V" ,2);
  } else {
      strncpy(voltDecade, "mV" ,2);
  }
}

void DialClass::handleKeypadDialog() {
  if (dialog==false) {
    return;
  }
  int screenWidth = 800;
  int screenHeight = 480;
  int width = 570;
  int height = 470;
  //int margin = 10;
  
  int startx, starty, endx, endy;
  startx=(screenWidth-width) / 2;
  starty=(screenHeight-height) / 2;
  endx=screenWidth - (screenWidth-width) / 2;
  endy=screenHeight - (screenHeight-height) / 2;
  
  // dialog black background
  GD.ColorA(230);
  GD.ColorRGB(0x000000);
  GD.Begin(RECTS);
  GD.Vertex2ii(startx, starty); 
  GD.Vertex2ii(endx, endy);
  
  // border
  GD.ColorRGB(0xaaaaaa);
  GD.Begin(LINE_STRIP);
  GD.LineWidth(32);
  GD.Vertex2ii(startx, starty); 
  GD.Vertex2ii(startx+width, starty); 
  GD.Vertex2ii(startx+width, starty+height); 
  GD.Vertex2ii(startx, starty+height); 
  GD.Vertex2ii(startx, starty); 
  
  
  // digit entry display
  GD.ColorRGB(0xaaaaaa);
  GD.Begin(LINE_STRIP);
  GD.LineWidth(16);
  GD.Vertex2ii(startx+10, starty+10); 
  GD.Vertex2ii(startx+width-10, starty+10); 
  GD.Vertex2ii(startx+width-10, starty+100); 
  GD.Vertex2ii(startx+10, starty+100); 
  GD.Vertex2ii(startx+10, starty+10); 

  //checkKeypress();
  
  GD.ColorA(255);

  mv = toMv();
  validate(mv);

 
  renderInput(error);

  renderKeypad();
}

void DialClass::checkKeypress() {

    int maxDigits = 7;

    //GD.get_inputs();

 
  if (GD.inputs.tag == KEYBOARD_CANCEL && dialog==true) {
    closedFn(set_or_limit, true);
      dialog = false;
    }
  else if (GD.inputs.tag == KEYBOARD_OK && dialog==true && error==false) {
    closedFn(set_or_limit, false);
    dialog = false;
  }

  

  
  if ( 1 == 1) {  // for now, allow to enter digits also when in error
  //if (error == false) {
    if (GD.inputs.tag != 0 && keydepressed==true) {
      keydepressed = false;
     // if (digits > 0 && GD.inputs.tag == KEYBOARD_OK) {
     // }


      if (GD.inputs.tag == KEYBOARD_TIMES10) {
        for (int i=0; i<digits;i++) {
          if (dialEntries[i] == KEYBOARD_COMMA) {
            if (i < digits -2) {
              dialEntries[i] = dialEntries[i+1];
              dialEntries[i+1] = KEYBOARD_COMMA;
            } else {
              dialEntries[i] = dialEntries[i+1];
              digits --;
            }
            break;
          } 
        }
      }

      
      if (GD.inputs.tag == KEYBOARD_DIVIDE10) {
        bool commaFound = false;
        for (int i=0; i<digits;i++) {
          if (dialEntries[i] == KEYBOARD_COMMA) {
            if (i>1) {
              dialEntries[i] = dialEntries[i-1];
              dialEntries[i-1] = KEYBOARD_COMMA;
            }
            else {
              dialEntries[i] = dialEntries[i-1];
              dialEntries[i-1] = KEYBOARD_COMMA;
              digits ++;
              for (int j = digits; j>=0;j--) {
                dialEntries[j] = dialEntries[j-1];
              }
              dialEntries[0] = 0;
            }
            commaFound = true;

            // don't have to many digits after comma
            if (digits > i + 2) {
              digits --;
            }
            
            break;
          }

        
          

          
        }

        if (!commaFound) {
          DEBUG.println("COMMA NOT FOUND !");
          dialEntries[digits] = dialEntries[digits-1];
          dialEntries[digits-1] = KEYBOARD_COMMA;
          digits ++;
        }



        
      }


      
      if (digits == 0 && GD.inputs.tag == KEYBOARD_COMMA) {
        dialEntries[digits++] = 0;
      }
        
      if (GD.inputs.tag== KEYBOARD_BACK) {
        if (digits>0) {
          digits --;
        }
      }
      if (GD.inputs.tag == KEYBOARD_COMMA) {
        bool alreadyHasComma = false;
        for (int i=0;i<digits;i++) {
          if (dialEntries[i] == KEYBOARD_COMMA) {
            alreadyHasComma = true;
            break;
          }
        }
        if (!alreadyHasComma) {
          dialEntries[digits++] = GD.inputs.tag;
        }
  
      }
      if (GD.inputs.tag >= KEYBOARD_1 && GD.inputs.tag <= KEYBOARD_9 && digits < maxDigits) {
        dialEntries[digits++] = GD.inputs.tag;
      }
      if (GD.inputs.tag == KEYBOARD_0 && digits < maxDigits ) {
        dialEntries[digits++] = 0;
      }
      if (GD.inputs.tag== KEYBOARD_UV) {
        strncpy(voltDecade, "uV" ,2);

      }
      if (GD.inputs.tag== KEYBOARD_MV) {
        strncpy(voltDecade, "mV" ,2);
      }
      if (GD.inputs.tag== KEYBOARD_V) {
        strncpy(voltDecade, "V" ,2);
      }

      if (GD.inputs.tag == KEYBOARD_PLUS_10MV) {
        
      }
      if (GD.inputs.tag == KEYBOARD_MINUS_10MV) {
        
      }
      
      if (GD.inputs.tag== KEYBOARD_CLR) {
        digits = 0;
      }
      if (set_or_limit == SET) {
        // Currently only allow positive current limit. It is used both for positive and negative current. Preliminary ?
        if (GD.inputs.tag== KEYBOARD_PLUSMINUS) {
          negative = !negative;
        }
      }

    } 
  }
  if (GD.inputs.tag == 0) {
    keydepressed = true;
  }
}
void DialClass::renderKeypad() {
    // keypad
  int x=190;
  int y=175;
  int spacing = 82;

  char symb[3] = "V";
  char symb_m[3] = "mV";
  char symb_u[3] = "uV";
  if ( (set_or_limit == SET and vol_cur_type == SOURCE_CURRENT) or  (set_or_limit == LIMIT and vol_cur_type == SOURCE_VOLTAGE))  {
    strncpy(symb,"A",1);
    strncpy(symb_m,"mA",2);
    strncpy(symb_u,"uA",2);
  }
  GD.Tag(1);
  transButton(x+0, y+0,18, "1", 31); 
  GD.Tag(2);
  transButton(x+spacing, y+0, 18, "2", 31);
  GD.Tag(3);
  transButton(x+spacing*2, y+0, 18, "3", 31);
  GD.Tag(KEYBOARD_V);
  transButton(x+spacing*3, y+0, 18, symb, 31);
  GD.Tag(KEYBOARD_BACK);
  transButton(x+spacing*4, y+0, 18, "<", 31);
  GD.Tag(KEYBOARD_PLUS_10MV);
  transButton(x+spacing*5, y+0, 18, "up", 29);
  
  GD.Tag(4);
  transButton(x+0, y+spacing, 18, "4", 31); 
  GD.Tag(5);
  transButton(x+spacing, y+spacing, 18, "5", 31); 
  GD.Tag(6);
  transButton(x+spacing*2, y+spacing, 18, "6", 31);
  GD.Tag(KEYBOARD_MV);
  transButton(x+spacing*3, y+spacing, 18, symb_m, 30);
  GD.Tag(KEYBOARD_CLR);
  transButton(x+spacing*4, y+spacing, 18, "Clr", 30);
  GD.Tag(KEYBOARD_MINUS_10MV);
  transButton(x+spacing*5, y+spacing, 18, "down", 29);

  
  GD.Tag(7);
  transButton(x+0, y+spacing*2, 18, "7", 31); 
  GD.Tag(8);
  transButton(x+spacing, y+spacing*2, 18, "8", 31); 
  GD.Tag(9);
  transButton(x+spacing*2, y+spacing*2, 18, "9", 31);
  GD.Tag(KEYBOARD_UV);
  transButton(x+spacing*3, y+spacing*2, 18, symb_u, 30);
  GD.Tag(KEYBOARD_DIVIDE10);
  transButton(x+spacing*4, y+spacing*2, 18, "/10", 30);
  
  GD.Tag(KEYBOARD_0);
  transButton(x+0, y+spacing*3, 18, "0",31); 
  GD.Tag(KEYBOARD_PLUSMINUS);
  transButton(x+spacing, y+spacing*3, 18, "+/-",30);
  GD.Tag(KEYBOARD_COMMA); 
  transButton(x+spacing*2, y+spacing*3, 18, ".",31);
  transButton(x+spacing*3, y+spacing*3, 18, " ", 31);
  GD.Tag(KEYBOARD_TIMES10);
  transButton(x+spacing*4, y+spacing*3, 18, "x10", 30);
  
  GD.Tag(KEYBOARD_CANCEL);
  transButton(x+spacing*5, y+spacing*2, 18, "Cancel", 28);
  if (!error && !warning) {
    GD.Tag(KEYBOARD_OK);
    transButton(x+spacing*5, y+spacing*3, 18, "OK", 31);
  }
}

void DialClass::renderInput(bool indicateError) {
    /* Show input values */
  int posx = 135;
  int posy = 8;
  if (indicateError) {
      GD.ColorRGB(0xFF0000);
  } else {
      GD.ColorRGB(COLOR_VOLT);
  }
  
  if (set_or_limit == SET) {
    // Currently dont show sign for current. Positive value is used both for neg and pos (sink/source). Preliminary ?
    if (negative == true) {
        GD.cmd_text(posx, posy, 1, 0, "-");
    } else {
        GD.cmd_text(posx, posy, 1, 0, "+");
    }
  }
  posx=posx+50;
  for (int i=0;i<digits;i++) {
    if (dialEntries[i] < KEYBOARD_COMMA) {
      GD.cmd_number(posx, posy, 1, 0, dialEntries[i]);
      posx=posx+50;
    }
    if (dialEntries[i]== KEYBOARD_COMMA) {
      GD.cmd_text(posx, posy, 1, 0, ".");
      posx=posx+20;
    }
    
    if (i==digits-1) {
      if ( (set_or_limit == SET and vol_cur_type == SOURCE_VOLTAGE) or  (set_or_limit == LIMIT and vol_cur_type == SOURCE_CURRENT))  {
          GD.cmd_text(posx+10, posy,1, 0, voltDecade);
      } else {
           strncpy(curDecade,"A ",2);
          if (strncmp(voltDecade,"uV",2) == 0) {
            strncpy(curDecade,"uA",2);
          } else if (strncmp(voltDecade,"mV",2) == 0) {
            strncpy(curDecade,"mA",2);
          }
          GD.cmd_text(posx+10, posy,1, 0, curDecade);
      }
    }
  }
}

void DialClass::showError(const char* text) {

  error = true;
  GD.ColorRGB(0xff0000);
  GD.cmd_text(160,107, 29, 0, text);
}

void DialClass::showWarning(const char* text) {
  warning = true;
  GD.ColorRGB(COLOR_VOLT);
  GD.cmd_text(160,107, 29, 0, text);
}

void DialClass::transButton(int x, int y, int sz, const char* label, int fontsize)
{
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
GD.cmd_text(x, y, fontsize, OPT_CENTER, label);
GD.ColorA(255);

}

int dialValidationTimer = millis();
void DialClass::validate(double mv) {
  // get millivolt number value and decimal value
  char buf[3+10];
  int numberValue, decimalValue;
  sprintf(buf, "%.*f", 3, mv);
  sscanf(buf, "%d.%d", &numberValue, &decimalValue);
//  DEBUG.print("mV:");
//  DEBUG.print(mv,5);
//  DEBUG.print(" Number value:");
//  DEBUG.print(numberValue);
//  DEBUG.print(", decimalValue:");
//  DEBUG.println(decimalValue);
//  DEBUG.flush();

  // get number of decimals in the display
  int decimalsAfterComma = -1;
  for (int i=0;i<digits;i++) {
    if (dialEntries[i] == KEYBOARD_COMMA) {
      decimalsAfterComma = digits -1 - i;
      break;
    }
  }
  if (vol_cur_type == SOURCE_VOLTAGE && set_or_limit == SET) {
    validateVoltage(mv, numberValue, decimalValue, decimalsAfterComma);
  } else if (vol_cur_type == SOURCE_CURRENT && set_or_limit == SET)  {
    validateCurrent(mv, numberValue, decimalValue, decimalsAfterComma);
  } else if (vol_cur_type == SOURCE_VOLTAGE && set_or_limit == LIMIT)  {
    validateCurrentLimit(mv, numberValue, decimalValue, decimalsAfterComma);
  } else if (vol_cur_type == SOURCE_CURRENT && set_or_limit == LIMIT)  {
    validateVoltageLimit(mv, numberValue, decimalValue, decimalsAfterComma);
  }
  
  
}


void DialClass::validateVoltage(double mv, int numberValue, int decimalValue, int decimalsAfterComma) {
  
  if (digits < 1) {
    showWarning("Please enter value");
    return;
  }
  if (decimalsAfterComma == 0){
    showWarning("Please enter a decimal after comma");
    return;
  }
  // Note that in the check below, mv is mv, independent
  // on which voltDecade is being show in the dislay
  
  if (strncmp(voltDecade,"V",1) == 0) {
    if (abs(mv) > 30000) {
      showError("Max voltage is 30V");
      return;
    }
    else if (decimalsAfterComma > 4) {
      showError("Max resolution in V range is 100uV");
      return;
    }
  }

  else if (strncmp(voltDecade,"mV",2) == 0) {
    if (abs(mv) > 30000) {
      showError("Max voltage is 30V");
      return;
    }
    else if (decimalsAfterComma > 2) {
      showError("Max resolution in mV range is 10uV");
      return;
    }
  }
 
  else if (strncmp(voltDecade,"uV",2) == 0) {
    if (numberValue > 999) {
      showError("Max voltage in uV range is 999mV");
      return;
    }
    else if (decimalValue < 100) {
      showError("Lower limit is 100uV");
      return;
    }
    else if (decimalsAfterComma > 0) {
      showError("nV not allowed");
      return;
    }
  }  
  error = false;
  warning = false;
}




void DialClass::validateVoltageLimit(double mv, int numberValue, int decimalValue, int decimalsAfterComma) {
  
  if (digits < 1) {
    showWarning("Please enter value");
    return;
  }
  if (decimalsAfterComma == 0){
    showWarning("Please enter a decimal after comma");
    return;
  }
  // Note that in the check below, mv is mv, independent
  // on which voltDecade is being show in the dislay
  
  if (strncmp(voltDecade,"V",1) == 0) {
    if (abs(mv) > 30000) {
      showError("Max voltage is 30V");
      return;
    }
    else if (decimalsAfterComma > 4) {
      showError("Max resolution in V range is 100uV");
      return;
    }
  }

  else if (strncmp(voltDecade,"mV",2) == 0) {
    if (abs(mv) > 30000) {
      showError("Max voltage is 30V");
      return;
    }
    else if (decimalsAfterComma > 2) {
      showError("Max resolution in mV range is 10uV");
      return;
    }
  }
 
  else if (strncmp(voltDecade,"uV",2) == 0) {
    if (numberValue > 999) {
      showError("Max voltage in uV range is 999mV");
      return;
    }
    else if (decimalValue < 100) {
      showError("Lower limit is 100uV");
      return;
    }
    else if (decimalsAfterComma > 0) {
      showError("nV not allowed");
      return;
    }
  }  
  error = false;
  warning = false;
}





void DialClass::validateCurrent(double mv, int numberValue, int decimalValue, int decimalsAfterComma) {


  if (digits < 1) {
    showWarning("Please enter value");
    return;
  }
  if (decimalsAfterComma == 0){
    showWarning("Please enter a decimal after comma");
    return;
  }
  // Note that in the check below, mv is mv, independent
  // on which voltDecade is being show in the dislay
  
  if (strncmp(voltDecade,"V",1) == 0) {
    if (abs(mv) > SETTINGS.max_current_1A_range()) {
      showError("Max current is 1.3A");
      return;
    }
    else if (decimalsAfterComma > 4) {
      showError("Max resolution in A range is 100uA");
      return;
    }
  }

  else if (strncmp(voltDecade,"mV",2) == 0) {
    if (abs(mv) > SETTINGS.max_current_1A_range()) {
      showError("Max current is 1300mA");
      return;
    }
    else if (decimalsAfterComma > 4 && mv < 10) {
      showError("Max resolution in mA range is 0.1uA");
      return;
    }
    else if (decimalsAfterComma > 2 && mv > 10) {
      showError("Max resolution in mA range is 10uA when >=10mA");
      return;
    }
  }
 
  else if (strncmp(voltDecade,"uV",2) == 0) {
    if (numberValue > 999) {
      showError("Max voltage in uA range is 999mA");
      return;
    }
    else if (decimalsAfterComma > 1) {
      showError("Max resolution in nA is 100nA");
      return;
    }
  }  
  error = false;
  warning = false;
}


void DialClass::validateCurrentLimit(double mv, int numberValue, int decimalValue, int decimalsAfterComma) {


  if (digits < 1) {
    showWarning("Please enter value");
    return;
  }
  if (decimalsAfterComma == 0){
    showWarning("Please enter a decimal after comma");
    return;
  }
  // Note that in the check below, mv is mv, independent
  // on which voltDecade is being show in the dislay
  
  if (strncmp(voltDecade,"V",1) == 0) {
    if (abs(mv) > SETTINGS.max_current_1A_range()) {
      showError("Max current is 1.3A");
      return;
    }
    else if (decimalsAfterComma > 4) {
      showError("Max resolution in A range is 100uA");
      return;
    }
  }

  else if (strncmp(voltDecade,"mV",2) == 0) {
    if (abs(mv) > SETTINGS.max_current_1A_range()) {
      showError("Max current is 1300mA");
      return;
    }
    else if (decimalsAfterComma > 2) {
      showError("Max resolution in mA range is 10uA ?????");
      return;
    }
  }
 
  else if (strncmp(voltDecade,"uV",2) == 0) {
    if (numberValue > 999) {
      showError("Max voltage in uA range is 999mA");
      return;
    }
    else if (decimalsAfterComma > 1) {
      showError("Max resolution in nA is 100nA");
      return;
    }
  }  
  error = false;
  warning = false;
}



double DialClass::toMv() {
/* calculate mv */
 int dec = 0;
 double sum = 0;
 for (int i=0;i<digits;i++) {
   if (dialEntries[i] == KEYBOARD_COMMA) {
     dec=1;
     i++;
   }
   if (dec==0) {
     if (sum==0){ // first
       sum = dialEntries[i];
     } else {
       sum = sum*10 + dialEntries[i];
     }
   } else {
      double decValue = dialEntries[i]/pow(10,dec);
      sum = sum + decValue;
      dec++;
   }
 }
 if (strncmp(voltDecade,"V",1) == 0) {
   sum=sum*1000;
 }
 if (strncmp(voltDecade,"uV",2) == 0) {
   sum=sum/1000;
 }
 return sum;
}

#include "tags.h"
#include "operations.h"
#include <stdint.h>
#include "digit_util.h"

class DialClass {

  void (*closedFn)(int set_or_limit, bool cancel);

  int screenWidth = 800;
  int screenHeight = 480;

 // Store memory for entered digits and symbols
  int dialEntries[15];
  // current number of digits in display
  int digits = 1;

  bool error;
  bool warning;

  int set_or_limit;
  
  bool keydepressed = true;
  char voltDecade[3] = "  ";
  char curDecade[3] = "  ";
  bool negative = false;
  bool dialog = false;
  double mv;
  int vol_cur_type;


  
public:
  void open(int type, int set_or_limit_, void (*closedFn)(int set_or_limit, bool cancel), int64_t micro);
  void close();
  void clear();
  bool isDialogOpen();
  void handleKeypadDialog();
  void checkKeypress();
  float getMv();
  void setMv(float mv);
  void init();
private:
  double toMv();
  void validate(double mv);
  void validateVoltage(double mv, int numberValue, int decimalValue, int decimalsAfterComma);
  void validateCurrent(double mv, int numberValue, int decimalValue, int decimalsAfterComma);
   void validateVoltageLimit(double mv, int numberValue, int decimalValue, int decimalsAfterComma);
  void validateCurrentLimit(double mv, int numberValue, int decimalValue, int decimalsAfterComma);
  void transButton(int x, int y, int sz, const char* label, int fontsize);
  void showError(const char* text);
  void showWarning(const char* text);
  void renderInput(bool valid);
  void renderKeypad();
};

extern DialClass SOURCE_DIAL;
extern DialClass LIMIT_DIAL;

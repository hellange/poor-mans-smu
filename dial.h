#define KEYBOARD_1 1
#define KEYBOARD_2 2
#define KEYBOARD_3 3
#define KEYBOARD_4 4
#define KEYBOARD_5 5
#define KEYBOARD_6 6
#define KEYBOARD_7 7
#define KEYBOARD_8 8
#define KEYBOARD_9 9
#define KEYBOARD_0 10

#define KEYBOARD_COMMA 11
#define KEYBOARD_UV 12
#define KEYBOARD_MV 13
#define KEYBOARD_V 14
#define KEYBOARD_BACK 15
#define KEYBOARD_CLR 16

#define KEYBOARD_PLUSMINUS 17
#define KEYBOARD_DIVIDE10 18
#define KEYBOARD_TIMES10 19

#define KEYBOARD_OK 50
#define KEYBOARD_CANCEL 51

#define BUTTON_VOLT_SET 100





class DialClass {

  int screenWidth = 800;
  int screenHeight = 480;

  // Store memory for entered digits and symbols
  int dialEntries[15];
  // current number of digits in display
  int digits; 

  bool error;
  bool warning;
  
  bool keydepressed = true;
  char* voltDecade = "  ";
  bool negative = false;
  bool dialog = false;
  double mv;
  
public:
  void open();
  void clear();
  void handleKeypad();
  float getMv();
private:
  double toMv();
  void validate(double mv);
  void transButton(int x, int y, int sz, char* label, int fontsize);
  void showError(char* text);
  void showWarning(char* text);
  void renderInput(bool valid);
  void renderKeypad();
};

extern DialClass DIAL;


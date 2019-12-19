#include "operations.h"

class MainMenuClass {

  void (*closedMainMenuFn)(int type);
  
  private:
    int scrollMainMenu = 0;
    int functionType = SOURCE_DC;
  public:
    bool active = false;
    int scrollMainMenuDir = 0;
    void open(void (*closedMenuFn)(int type));
    void close();
    void handleButtonAction(int tag);
    void render();
  };

extern MainMenuClass MAINMENU;

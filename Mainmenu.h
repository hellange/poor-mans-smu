#include "operations.h"

class MainMenuClass {

  void (*closedMainMenuFn)(FUNCTION_TYPE type);
  
  private:
    int scrollMainMenu = 0;
    FUNCTION_TYPE functionType = SOURCE_DC_VOLTAGE;
  public:
    bool active = false;
    int scrollMainMenuDir = 0;
    void open(void (*closedMenuFn)(FUNCTION_TYPE type));
    void close();
    void handleButtonAction(int tag);
    void render();
  };

extern MainMenuClass MAINMENU;

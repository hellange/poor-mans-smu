class MainMenuClass {

  void (*closedMainMenuFn)(int type);
  
  private:
    int scrollMainMenu = 0;

  public:
    bool active = false;
    int scrollMainMenuDir = 0;
    void open(void (*closedMenuFn)(int type));
    void close();
    void handle();
  };

extern MainMenuClass MAINMENU;

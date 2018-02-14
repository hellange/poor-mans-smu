class VoltDisplayClass {
public:
  void render(int x, int y, float rawMv);
  void boldText(int x, int y, char* text);
  void boldNumber(int x, int y, int digits, int number);
};

extern VoltDisplayClass VOLT_DISPLAY;


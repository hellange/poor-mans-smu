class CurrentDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMa);
  void renderSet(int x, int y, float rawMa);
    void boldText(int x, int y, char* text);
  void boldNumber(int x, int y, int digits, int number);
private:
  char *sign = '+';
};

extern CurrentDisplayClass CURRENT_DISPLAY;

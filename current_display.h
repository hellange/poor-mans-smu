class CurrentDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMa, bool compliance);
  void renderOverflow(int x, int y);
  void renderSet(int x, int y, float rawMa);
  void boldText(int x, int y, const char *text);
  void boldNumber(int x, int y, int digits, int number);
private:
  char sign[2] = "+";
};

extern CurrentDisplayClass CURRENT_DISPLAY;

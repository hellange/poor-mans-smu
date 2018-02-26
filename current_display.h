class CurrentDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMa);
  void renderSet(int x, int y, float rawMa);
private:
  char *sign = '+';
};

extern CurrentDisplayClass CURRENT_DISPLAY;

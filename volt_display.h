class VoltDisplayClass {
public:
  void renderMeasured(int x, int y, float rawMv);
  void renderSet(int x, int y, float rawMv);
  void boldText(int x, int y, char* text);
  void boldNumber(int x, int y, int digits, int number);
  void separate(int *v, int *mv, int *uv, float raw);

private:
  char *sign = '+';
};

extern VoltDisplayClass VOLT_DISPLAY;


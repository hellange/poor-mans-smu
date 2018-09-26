class StatsClass {
private:
  int endPtr = 0;
  float value[100];
  float uispan;
  int nrOfTrendPoints = 75;
  void updateMinMax(float rawMv);

public:
  void renderTrend(int x, int y, bool small);
  void addSample(float rawMv);
  float minimum;
  float maximum;
  float visibleMax;
  float visibleMin;
  float span;
};




class StatsClass {
private:
  int endPtr = 0;
  float value[200];
  float uispan;
  int nrOfTrendPoints;

public:
  void renderTrend(int x, int y, int samples, bool small);
  void addSample(float rawMv);
  float minimum;
  float maximum;
  float visibleMax;
  float visibleMin;
  float span;
};




class StatsClass {
#define nrOfTrendPoints 75
private:
  int endPtr = 0;
  float value[nrOfTrendPoints];
  float uispan;

public:
  void renderTrend(int x, int y);
  void addSample(float rawMv);
  float minimum;
  float maximum;
  float visibleMax;
  float visibleMin;
  float span;
};

extern StatsClass STATS;

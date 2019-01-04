
class StatsClass {
  #define MAX_SAMPLES_BEFORE_STORE 5

private:
  int endPtr = 0;
  float value[75];
  float valueExt[75][2];  // max min value for each
  float uispan;
  int nrOfTrendPoints = 75;
  void updateMinMax();
  int maxSamplesBeforeStore = MAX_SAMPLES_BEFORE_STORE;
  int prelimSamplesCounter = 0;
  float prelimBuffer[MAX_SAMPLES_BEFORE_STORE];
public:
  void init();
  void renderTrend(int x, int y);
  void addSample(float rawMv);
  float rawMv;
  float minimum;
  float maximum;
  float visibleMax;
  float visibleMin;
  float span;
};




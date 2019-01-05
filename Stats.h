
class StatsClass {
  #define MAX_SAMPLES_BEFORE_STORE 5
#define MAX_FULL_SAMPLES 85
private:
  int endPtr = 0;
  float value[MAX_FULL_SAMPLES];
  float valueExt[MAX_FULL_SAMPLES][2];  // max min value for each
  float uispan;
  int nrOfTrendPoints = MAX_FULL_SAMPLES;
  void updateMinMax();
  int maxSamplesBeforeStore = MAX_SAMPLES_BEFORE_STORE;
  int prelimSamplesCounter = 0;
  float prelimBuffer[MAX_SAMPLES_BEFORE_STORE];
public:
  void init();
  void renderTrend(int x, int y, bool limitDetails);
  void addSample(float rawMv);
  float rawMv;
  float minimum;
  float maximum;
  float visibleMax;
  float visibleMin;
  float span;
};





class StatsClass {
#define MAX_SAMPLES_BEFORE_STORE 5
#define MAX_FULL_SAMPLES 75
private:
  float undefinedValue = 1000000; // just a way to indicate that value is not set...
  int endPtr = 0;
  float value[MAX_FULL_SAMPLES];
  float valueExt[MAX_FULL_SAMPLES][2];  // max min value for each
  float uispan;
  int nrOfTrendPoints = MAX_FULL_SAMPLES;
  void updateMinMax();
  int maxSamplesBeforeStore = MAX_SAMPLES_BEFORE_STORE;
  int prelimSamplesCounter = 0;
  float prelimBuffer[MAX_SAMPLES_BEFORE_STORE];
  float visibleMaxSlow = -undefinedValue; 
  float visibleMinSlow = undefinedValue;
  int type;
  
  const int noOfBins = 21;
public:
  void init(int type);
  void renderTrend(int x, int y, bool limitDetails);
  void renderHistogram(int x, int y, bool limitDetails);
  void addSample(float rawValue);
  float rawValue;
  float minimum;
  float maximum;
  float minimumMean;
  float maximumMean;
  float visibleMax;
  float visibleMin;
  float span;
  
};




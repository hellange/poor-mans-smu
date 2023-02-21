
#ifndef STATS_H
#define STATS_H

class StatsClass {

#define MAX_FULL_SAMPLES 75
#define MAX_PRELIMBUFFER_SIZE 1000 
private:

  float prelimBuffer[MAX_PRELIMBUFFER_SIZE];
  int maxSamplesBeforeStore = 5;

  float undefinedValue = 1000000; // just a way to indicate that value is not set...
  int endPtr = 0;
  float value[MAX_FULL_SAMPLES];
  float valueExt[MAX_FULL_SAMPLES][2];  // max min value for each
  float uispan;
  int nrOfTrendPoints = MAX_FULL_SAMPLES;
  void updateMinMax();
  int prelimSamplesCounter = 0;
  float visibleMaxSlow = -undefinedValue; 
  float visibleMinSlow = undefinedValue;
  int type;
  const int noOfBins = 21;
public:
  void init(int type);
  void clearBuffer();
  void renderTrend(int x, int y, bool limitDetails);
  void renderHistogram(int x, int y, bool limitDetails);
  void addSample(float rawValue);
  void setNrOfSamplesBeforeStore(int n);
  int getNrOfSamplesBeforeStore();

  float rawValue;
  float minimum;
  float maximum;
  float minimumMean;
  float maximumMean;
  float visibleMax;
  float visibleMin;
  float span;
  
};

extern StatsClass V_STATS;
extern StatsClass C_STATS;
#endif
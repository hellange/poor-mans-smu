#ifndef SIMPLE_STATS_H
#define SIMPLE_STATS_H
class SimpleStatsClass {

 private:

  float undefinedValue = 1000000; // just a way to indicate that value is not set...
  
public:
  void init();
  void registerValue(float value);
  void clear();
  
  float minimum;
  float maximum;
  float samples;
  
};

extern SimpleStatsClass SIMPLE_STATS;

#endif
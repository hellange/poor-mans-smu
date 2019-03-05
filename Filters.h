
class FiltersClass {

#define FILTER_LENGTH 100

private:
  float meanRaws[FILTER_LENGTH];
public:
  void init();
  float updateMean(float v);
  float mean;
};




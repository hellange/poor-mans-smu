
class FiltersClass {

#define FILTER_LENGTH 10

private:
  float meanRaws[FILTER_LENGTH];
public:
  void init();
  float updateMean(float v);
  float mean;
};

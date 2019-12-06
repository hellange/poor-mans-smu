
class FiltersClass {

#define FILTER_MAX_LENGTH 1000

private:
  float meanRaws[FILTER_MAX_LENGTH];
public:
  void init();
  int filterSize;

  float updateMean(float v);
  float mean;
  void setFilterSize(int size);
};

extern FiltersClass V_FILTERS;
extern FiltersClass C_FILTERS;

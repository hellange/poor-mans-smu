#include "Filters.h"
#include "Arduino.h"

FiltersClass V_FILTERS;
FiltersClass C_FILTERS;

void FiltersClass::init(int id_)
{
  id = id_;
  filterSize = 5; // start out with a bit of filtering
  Serial.print("Set initial filter length to:");
  Serial.println(filterSize);
  for (int i = 0; i < FILTER_MAX_LENGTH; i++)
  {
    meanRaws[i] = 0.0;
  }
}

void FiltersClass::setFilterSize(int size)
{
  if (size < 1)
  {
    Serial.println("WARNING: Set to minium allowed filter size:");
    filterSize = 1;
  }
  else if (size <= FILTER_MAX_LENGTH)
  {
    filterSize = size;
    Serial.print("Set filter size to:");
  }
  else
  {
    Serial.println("WARNING: Reduced filter size to max allowed:");
  }
  Serial.println(filterSize);
}

float FiltersClass::updateMean(float v, bool moving)
{
  int fz = filterSize;

  // if you dont want moving average value. This will update mean value after a full <filterSize> set of sampleshave been collected.
  // This is nice if you dont want the display digits to update too quickly.
  // TODO: Use correct terminology from statistics...
  if (!moving)
  {
    if (mean_samples < filterSize)
    {
      meanRaws[mean_samples] = v;
      mean_samples++;
      return mean;
    }
    else
    {
      mean_samples = 0;
    }
  }
  //  if (!moving) {
  //    if (mean_samples < filterSize) {
  //      meanRaws[mean_samples] = v;
  //      mean_samples ++;
  //    } else {
  //      // Do the actual calculation from relevant samples
  //      float tot = 0;
  //      for (int i=0;i<fz;i++){
  //        tot = tot + meanRaws[i];
  //      }
  //      mean = tot / fz;
  //      mean_samples = 0;
  //    }
  //    return mean;
  //  }

  // shift everything one step
  // TODO: Improve to use pointer into latest field instead of having to shift all entries...
  for (int i = fz - 1; i > 0; i--)
  {
    meanRaws[i] = meanRaws[i - 1];
  }
  meanRaws[0] = v;

  // Do the actual calculation from relevant samples
  float tot = 0;
  for (int i = 0; i < fz; i++)
  {
    tot = tot + meanRaws[i];
  }
  mean = tot / fz;

  return mean;
}

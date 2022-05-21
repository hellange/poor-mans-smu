#include <inttypes.h>
#include <SPI.h>

#include "GD2.h"
#include "digit_util.h"


#define MAX_SAMPLES_IN_DISPLAY_BUFFER 300

class LogDataWithTimeStamp {
  public:
    float value;
    uint32_t timestamp;
};

class LoggerClass {

 private:
  static bool encButtonScroll;// = true;
  static bool encButtonZoom;// = false;

  bool notAvailable = true;
  uint32_t *memory_begin, *memory_end;
  bool full;
  bool empty;
  float undefinedValue = 999999999.0; // just a way to indicate that value is not set...
  float loggerMaxValue = -10000.0;
  float loggerMinValue = 10000.0;

  void fillBuffer(int nrOfValues);
  uint32_t waitLogStart = 5000; // don't care about the first samples at startup...

  // number of samples in the history. Each sample takes 8bytes, so this 
  // valus should be adjusted according to available ram.
  // For a 8Mbyte extra Teensy 4.1 RAM this means buffer ot 1M samples max.
  uint32_t maxSize = 1000000; 
  
  // number of samples to be shown in display
  // This value will be changed dynamically by the user
  // Not that the buffer must at 
  int nrOfWindowSamples = MAX_SAMPLES_IN_DISPLAY_BUFFER; 
  
  // makes it possible to let one point in graph represent value of several samples, for example using mean value...
  int samplesPrViewPoint = 1; 


  int totalSamplesCounted;  // counts up for each sample (independent on buffer size!)


public:
  void init();
  void registerValue(LogDataWithTimeStamp logData);
  void registerValue2(float value);

  void clear();
  void printBuffer();
  void getLastSamples(LogDataWithTimeStamp *buffer, int bufferSize, int samplesPr);
  void render();
  void loop();
  void updateViewData(int samplesPr);
  
  float percentageFull;

  float minimum;
  float maximum;
  uint32_t samples;
  uint32_t address;

  uint32_t scrollAddress;
  boolean autoScrolling;

  static int rotary;
  static int scrollRotary;
  static void rotaryChangedFn(float changeVal);
  static void rotarySwitchFn(int key, bool quickPress, bool holdAfterLongPress, bool releaseAfterLongPress);
  
};

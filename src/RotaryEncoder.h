#include "Encoder.h"

class RotaryEncoderClass {

  void (*changedFn)(float value);

  private:
    
    // Change these pin numbers to the pins connected to your encoder.
    //   Best Performance: both pins have interrupt capability
    //   Good Performance: only the first pin has interrupt capability
    //   Low Performance:  neither pin has interrupt capability
    //Encoder knobRight(7, 8);
    
    long positionLeft  = 0; //-999;
    //long positionRight = -999;
    
    int millisSinceLastStep = millis();
    int millisSinceLastVoltageCommit = millis();
    boolean ready = false;
  public:
    void init(void (*changedFn)(float value));
    void handle(bool use100uVSetResolution);
};


extern RotaryEncoderClass ROTARY_ENCODER;

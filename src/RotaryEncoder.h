//#include "Encoder.h"
#include "EncoderTool.h" // Seems to handle debounce from mechanical imperfect knob better that Encoder 
                         // Ref.:https://github.com/luni64/EncoderTool

class RotaryEncoderClass {

  void (*changedFn)(float value);

  private:
    
    EncoderTool::Encoder knobLeft;
    long positionLeft  = 0;
    int millisAtLastStep = millis();
    int millisAtLastVoltageCommit = millis();
    boolean ready = false;
    
  public:
    void init(void (*changedFn)(float value));
    void handle(bool reduceResolution);
    bool stepless_dynamic = true;  // decide if the dynamic speed shall be directly dependent on rotation speed or if there shall just be a few different speeds 

};

extern RotaryEncoderClass ROTARY_ENCODER;

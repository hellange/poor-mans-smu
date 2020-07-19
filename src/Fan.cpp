// Ref also https://forum.pjrc.com/threads/34606-4-pin-PWM-Fan-Controller-(port-form-UNO-to-teensy-3-2)

#include "Fan.h"
#include "Arduino.h"
FanClass FAN;
volatile uint32_t _PWMFanWidth;

void FanClass::init()
{
  int fanPin = 23;     // TODO: Move to global definitions
  int fanSpeedPin = 2; // TODO: Move to global definitions
  pinMode(fanPin, OUTPUT);
  digitalWriteFast(9, LOW);
  pinMode(fanSpeedPin, INPUT);
  attachInterrupt(fanSpeedPin, fanSpeedInterruptHandler, CHANGE);

  // For now, just hardcode a value for testing
  analogWriteFrequency(fanPin, 10000);
  analogWrite(fanPin, 100);
}

int FanClass::getFanWidth()
{
  int fanSpeed = (int)_PWMFanWidth;
  return fanSpeed;
}

// TODO: Test !!!
uint16_t FanClass::getPWMFanRPM()
{
  __disable_irq();
  uint32_t width = _PWMFanWidth;
  //_PWMFanWidth=0UL;
  __enable_irq();
  if (width == 0UL)
  {
    return 0;
  }

  // width seems to be the ms period of a complete pulse (includes on and off)
  // 4 wire fan speed defines 2 pulses pr revolution.
  return (uint16_t)(1000000 / width) / 2; //TODO: check this calculation. Does not agree with the original one commented out below
  //return (uint16_t)(30000000UL/width);
}

void FanClass::fanSpeedInterruptHandler()
{
  static uint32_t timestamp;
  static uint8_t state = 0;
  uint8_t current = (CORE_PIN2_PINREG & _BV(CORE_PIN2_BIT)) ? 1 : 0; //check tach pin is high or low
  if (state == current)
    return;
  state = current;
  if (state)
  {
    uint32_t t = micros();
    _PWMFanWidth = t - timestamp;
    timestamp = t;
  }
}

uint16_t FanClass::getRPMValueFiltered()
{
  if (timeSinceLastRPMReadout + 1000 < millis())
  {
    storedPWMFanSpeed = getPWMFanRPM();
    timeSinceLastRPMReadout = millis();
  }
      return storedPWMFanSpeed;
}

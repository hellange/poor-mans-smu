//EEPROMAnything is taken from here: http://www.arduino.cc/playground/Code/EEPROMWriteAnything

#ifndef EEPROMAnything_h
#define EEPROMAnything_h

#include "EEPROM.h"
#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    ee = ee + EA_ANYTHING; // offst to eeprom start for anything
    const byte* p = (const byte*)(const void*)&value;
    uint64_t i;
    for (i = 0; i < sizeof(value); i++)
        EEPROM.update(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    ee = ee + EA_ANYTHING; // offset to eeprom start for anything
    byte* p = (byte*)(void*)&value;
    uint64_t i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}



#endif
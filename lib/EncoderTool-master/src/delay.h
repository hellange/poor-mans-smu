#pragma once

#include "Arduino.h"

namespace EncoderTool
{

#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)

    inline void delay50ns()
    {
        delayNanoseconds(10);
    }

#else

    inline void delay50ns()
    {
        delayMicroseconds(1);
    }

#endif
} // namespace TeensyEncoderTool
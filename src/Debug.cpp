#include "Debug.h"

DebugClass DEBUG;

void DebugClass::println(char const *c) {
   if (enabled == true) {
        Serial.println(c);
   }
}

void DebugClass::println(float f) {
    if (enabled == true) {
        Serial.println(f);
    }
}

void DebugClass::println(long a, int base) {
    if (enabled == true) {
        Serial.println(a, base);
    }
}

void DebugClass::println(unsigned long a, int base) {
    if (enabled == true) {
        Serial.println(a, base);
    }
}

void DebugClass::println(float f, int base) {
    if (enabled == true) {
        Serial.println(f, base);
    }
}

void DebugClass::println(int a, int base) {
    if (enabled == true) { 
        Serial.println(a, base);
    }
}

void DebugClass::println() { 
    if (enabled) {
        Serial.println();
    }
}


void DebugClass::printchar(char a) {
    if (enabled == true) {
        Serial.print(a);
    }
}

void DebugClass::print(int a, int base) {
    if (enabled == true) {
        Serial.print(a, base);
    }
}

void DebugClass::print(long a, int base) {
    if (enabled == true) {
        Serial.print(a, base);
    }
}

void DebugClass::print(char const *c) {
    if (enabled == true) {
        Serial.print(c);
    }
}

void DebugClass::print(float f) {
    if (enabled == true) {
        Serial.print(f);
    }
}

void DebugClass::print(float f, int nr) {
    if (enabled == true) {
        Serial.print(f, nr);
    }
}

void DebugClass::print(double d, int nr) {
    if (enabled == true) {
        Serial.print(d, nr);
    }
}

void DebugClass::printf(const char *format, uint8_t n) {
    if (enabled == true) {
        Serial.printf(format, n);
    }
}

void DebugClass::flush() {
    if (enabled == true) {
        Serial.flush();
    }
}

void DebugClass::printError(int16_t errorNum) {
    if (enabled == true) {
        Serial.print(F("Error("));
        Serial.print(errorNum);
        Serial.println(")");
    }
}
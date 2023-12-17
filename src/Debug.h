#ifndef DEBUG_H
#define DEBUG_H

#include <SPI.h>

  
class DebugClass {
      bool enabled = true;

  public:
    void println(char const *c);
    void println(float f);
    void println(long a, int base);
    void println(unsigned long a, int base);
    void println(float f, int nr);
    void println(int a, int base);
    void println();

    void printchar(char a);
    void print(char const *c);
    void print(int a, int base);
    void print(long a, int base);

    void print(float f);
    void print(float f, int base);
    void print(double d, int nr);

    void printf(const char *format, uint8_t n);

    void flush();
    
    void printError(int16_t errorNum);
};

extern DebugClass DEBUG;


#endif
/* serialRAM library code is placed under the MIT license
* Copyright (c) 2015 Frank Bösing
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "SerialRAM.h"

#define CSCONFIG() pinMode(RAM_CS, OUTPUT)
#define CSASSERT() digitalWriteFast(RAM_CS, LOW)
#define CSRELEASE() digitalWriteFast(RAM_CS, HIGH)
#define SPICONFIG SPISettings(10000000, MSBFIRST, SPI_MODE0)


void ram_init(void)
{
   CSCONFIG();
   CSRELEASE();
   ram_writeRDMR(RAM_SEQUENTIALMODE);
}

//Read Mode Register
int ram_readRDMR(void)
{
  int ret;
  SPI.beginTransaction(SPICONFIG);
  CSASSERT();
  SPI.transfer(RAM_INSTR_RDMR);
  ret = SPI.transfer(0);
  CSRELEASE();
  SPI.endTransaction();
  return ret;
}

//Write Mode Register
void ram_writeRDMR(uint8_t mode)
{
  SPI.beginTransaction(SPICONFIG);
  CSASSERT();
  SPI.transfer( RAM_INSTR_WRMR);
  SPI.transfer(mode);
  CSRELEASE();
  SPI.endTransaction();
}

void ram_read(uint8_t *buf, uint32_t addr, uint32_t len)
{ //TODO: FIFO
  SPI.beginTransaction(SPICONFIG);
  CSASSERT();  
  SPI.transfer(RAM_INSTR_READ);
  SPI.transfer(addr>>16);
  SPI.transfer(addr>>8);
  SPI.transfer(addr);
  while(len--) {
    *buf++ = SPI.transfer(0);
    }
  CSRELEASE();
  SPI.endTransaction();    
}

void ram_write(uint8_t *buf, uint32_t addr, uint32_t len)
{ //TODO: FIFO
  SPI.beginTransaction(SPICONFIG);
  CSASSERT();  
  SPI.transfer(RAM_INSTR_WRITE);
  SPI.transfer(addr>>16);
  SPI.transfer(addr>>8);
  SPI.transfer(addr);
  while(len--) {
    SPI.transfer(*buf++);
    }
  CSRELEASE();
  SPI.endTransaction();    
}

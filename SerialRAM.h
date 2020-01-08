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

#ifndef _SerialRAM_h
#define _SerialRAM_h

#include <SPI.h>

#ifndef RAM_CS
#define RAM_CS 5//CHIPSELECT
#endif

//Instructions
#define RAM_INSTR_READ      0x03
#define RAM_INSTR_WRITE     0x02
#define RAM_INSTR_EDIO      0x3B
#define RAM_INSTR_EQIO      0x38
#define RAM_INSTR_RSTIO     0xFF
#define RAM_INSTR_RDMR      0x05
#define RAM_INSTR_WRMR      0x01

#define RAM_BYTEMODE        0x00
#define RAM_PAGEMODE        0x80
#define RAM_SEQUENTIALMODE  0x40

void ram_init(void);

int ram_readRDMR(void);
void ram_writeRDMR(uint8_t mode);

void ram_read(uint8_t *buf, uint32_t addr, uint32_t len);
void ram_write(uint8_t *buf, uint32_t addr, uint32_t len);
#endif

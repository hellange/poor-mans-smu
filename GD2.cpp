// 
// Gameduino2 code, minimized to include only what I use and to be possible
// to compile for NodeMCU
//
#include <Arduino.h>
#include "SPI.h"
#if !defined(__SAM3X8E__)
#include "EEPROM.h"
#endif
#define VERBOSE       2
#include "GD2.h"

#define BOARD_FTDI_80x    0
#define BOARD_GAMEDUINO23 1
#define BOARD_EVITA_0     2
#define BOARD_VM810C      3

#define BOARD         BOARD_VM810C 

#ifdef DUMPDEV
#include <assert.h>
#include "transports/dump.h"
#endif

byte ft8xx_model;

#if defined(ARDUINO)
#include "wiring_gd.h"
#endif

////////////////////////////////////////////////////////////////////////


void xy::set(int _x, int _y)
{
  x = _x;
  y = _y;
}

void xy::rmove(int distance, int angle)
{
  x -= GD.rsin(distance, angle);
  y += GD.rcos(distance, angle);
}

int xy::angleto(class xy &other)
{
  int dx = other.x - x, dy = other.y - y;
  return GD.atan2(dy, dx);
}

void xy::draw(byte offset)
{
  GD.Vertex2f(x - PIXELS(offset), y - PIXELS(offset));
}

int xy::onscreen(void)
{
  return (0 <= x) &&
         (x < PIXELS(GD.w)) &&
         (0 <= y) &&
         (y < PIXELS(GD.h));
}

class xy xy::operator+=(class xy &other)
{
  x += other.x;
  y += other.y;
  return *this;
}

int xy::nearer_than(int distance, xy &other)
{
  int lx = abs(x - other.x);
  if (lx > distance)
    return 0;
  int ly = abs(y - other.y);
  if (ly > distance)
    return 0;

  // trivial accept: 5/8 is smaller than 1/sqrt(2)
  int d2 = (5 * distance) >> 3;
  if ((lx < d2) && (ly < d2))
    return 1;

  return ((lx * lx) + (ly * ly)) < (distance * distance);
}


////////////////////////////////////////////////////////////////////////

static GDTransport GDTR;

GDClass GD;

////////////////////////////////////////////////////////////////////////

// The GD3 has a tiny configuration EEPROM - AT24C01D
// It is programmed at manufacturing time with the setup
// commands for the connected panel. The SCL,SDA lines
// are connected to thye FT81x GPIO0, GPIO1 signals.
// This is a read-only driver for it.  A single method
// 'read()' initializes the RAM and reads all 128 bytes
// into an array.

class ConfigRam {
private:
  uint8_t gpio, gpio_dir, sda;
  void set_SDA(byte n)
  {
    if (sda != n) {
      GDTR.__wr16(REG_GPIO_DIR, gpio_dir | (0x03 - n));    // Drive SCL, SDA low
      sda = n;
    }
  }

  void set_SCL(byte n)
  {
    GDTR.__wr16(REG_GPIO, gpio | (n << 1));
  }

  int get_SDA(void)
  {
    return GDTR.__rd16(REG_GPIO) & 1;
  }

  void i2c_start(void) 
  {
    set_SDA(1);
    set_SCL(1);
    set_SDA(0);
    set_SCL(0);
  }

  void i2c_stop(void) 
  {
    set_SDA(0);
    set_SCL(1);
    set_SDA(1);
    set_SCL(1);
  }

  int i2c_rx1()
  {
    set_SDA(1);
    set_SCL(1);
    byte r = get_SDA();
    set_SCL(0);
    return r;
  }

  void i2c_tx1(byte b)
  {
    set_SDA(b);
    set_SCL(1);
    set_SCL(0);
  }

  int i2c_tx(byte x)
  {
    for (byte i = 0; i < 8; i++, x <<= 1)
      i2c_tx1(x >> 7);
    return i2c_rx1();
  }

  int i2c_rx(int nak)
  {
    byte r = 0;
    for (byte i = 0; i < 8; i++)
      r = (r << 1) | i2c_rx1();
    i2c_tx1(nak);
    return r;
  }

public:
  void read(byte *v)
  {
    GDTR.__end();
    gpio = GDTR.__rd16(REG_GPIO) & ~3;
    gpio_dir = GDTR.__rd16(REG_GPIO_DIR) & ~3;
    sda = 2;

    // 2-wire software reset
    i2c_start();
    i2c_rx(1);
    i2c_start();
    i2c_stop();

    int ADDR = 0xa0;

    i2c_start();
    if (i2c_tx(ADDR))
      return;
    if (i2c_tx(0))
      return;

    i2c_start();
    if (i2c_tx(ADDR | 1))
      return;
    for (int i = 0; i < 128; i++) {
      *v++ = i2c_rx(i == 127);
      // Serial.println(v[-1], DEC);
    }
    i2c_stop();
    GDTR.resume();
  }
};

void GDClass::flush(void)
{
  GDTR.flush();
}

void GDClass::swap(void) {
  Display();
  cmd_swap();
  cmd_loadidentity();
  cmd_dlstart();
  GDTR.flush();
#ifdef DUMPDEV
  GDTR.swap();
#endif
}


void GDClass::begin(uint8_t options) {
#if defined(ARDUINO)
  GDTR.begin0();
#endif
  GDTR.begin1();

  rseed = 0x77777777;

#if 1
  Serial.println("ID REGISTER:");
  Serial.println(GDTR.rd(REG_ID), HEX);
#endif


  GDTR.wr(REG_PWM_DUTY, 0);
  GDTR.wr(REG_GPIO_DIR, 0x83);
  GDTR.wr(REG_GPIO, GDTR.rd(REG_GPIO) | 0x80);

// configuration based on info in thread:
// http://gameduino2.proboards.com/thread/270/gameduino-3
// works for http://www.buydisplay.com/default/7-tft-lcd-touch-screen-display-module-800x480-for-mp4-gps-tablet-pc
#if (BOARD == BOARD_VM810C)
  GD.wr32(REG_HCYCLE, 900);//548
  GD.wr32(REG_HOFFSET, 43);
  GD.wr32(REG_HSIZE, 800);
  GD.wr32(REG_HSYNC0, 0);
  GD.wr32(REG_HSYNC1, 41);
  GD.wr32(REG_VCYCLE, 500);
  GD.wr32(REG_VOFFSET, 12);
  GD.wr32(REG_VSIZE, 480);
  GD.wr32(REG_VSYNC0, 0);
  GD.wr32(REG_VSYNC1, 10);
  GD.wr32(REG_DITHER, 1);
  GD.wr32(REG_PCLK_POL, 1);//1
  GD.wr32(REG_PCLK, 3);//5
  GD.wr(REG_ROTATE, 0);
  GD.wr(REG_SWIZZLE, 0);//3 for GD2
#endif

  w = GDTR.rd16(REG_HSIZE);
  h = GDTR.rd16(REG_VSIZE);
  Serial.print("Width:");
  Serial.println(w);
  Serial.print("Height");
  Serial.println(h);

  Clear(); swap();
  Clear(); swap();
  Clear(); swap();
  cmd_regwrite(REG_PWM_DUTY, 128);
  GD.flush();
}

void GDClass::align(byte n) {
  while ((n++) & 3)
    GDTR.cmdbyte(0);
}

void GDClass::cH(uint16_t v) {
  GDTR.cmdbyte(v & 0xff);
  GDTR.cmdbyte((v >> 8) & 0xff);
}

void GDClass::ch(int16_t v) {
  cH((uint16_t)v);
}

void GDClass::cI(uint32_t v) {
  GDTR.cmd32(v);
}

void GDClass::cFFFFFF(byte v) {
  union {
    uint32_t c;
    uint8_t b[4];
  };
  b[0] = v;
  b[1] = 0xff;
  b[2] = 0xff;
  b[3] = 0xff;
  GDTR.cmd32(c);
}

void GDClass::ci(int32_t v) {
  cI((uint32_t) v);
}

void GDClass::cs(const char *s) {
  int count = 0;
  while (*s) {
    char c = *s++;
    GDTR.cmdbyte(c);
    count++;
  }
  GDTR.cmdbyte(0);
  align(count + 1);
}

void GDClass::copy(const  uint8_t *src, int count) {
  byte a = count & 3;
  while (count--) {
    GDTR.cmdbyte(pgm_read_byte_near(src));
    src++;
  }
  align(a);
}

void GDClass::copyram(byte *src, int count) {
  byte a = count & 3;
  GDTR.cmd_n(src, count);
  align(a);
}

void GDClass::AlphaFunc(byte func, byte ref) {
  cI((9UL << 24) | ((func & 7L) << 8) | ((ref & 255L) << 0));
}
void GDClass::Begin(byte prim) {
  cI((31UL << 24) | prim);
}
void GDClass::BitmapHandle(byte handle) {
  cI((5UL << 24) | handle);
}
void GDClass::BitmapLayout(byte format, uint16_t linestride, uint16_t height) {
  // cI((7UL << 24) | ((format & 31L) << 19) | ((linestride & 1023L) << 9) | ((height & 511L) << 0));
  union {
    uint32_t c;
    uint8_t b[4];
  };
  b[0] = height;
  b[1] = (1 & (height >> 8)) | (linestride << 1);
  b[2] = (7 & (linestride >> 7)) | (format << 3);
  b[3] = 7;
  cI(c);
}
void GDClass::BitmapSize(byte filter, byte wrapx, byte wrapy, uint16_t width, uint16_t height) {
  byte fxy = (filter << 2) | (wrapx << 1) | (wrapy);
  // cI((8UL << 24) | ((uint32_t)fxy << 18) | ((width & 511L) << 9) | ((height & 511L) << 0));
  union {
    uint32_t c;
    uint8_t b[4];
  };
  b[0] = height;
  b[1] = (1 & (height >> 8)) | (width << 1);
  b[2] = (3 & (width >> 7)) | (fxy << 2);
  b[3] = 8;
  cI(c);
  if (ft8xx_model) {
    b[0] = ((width >> 9) << 2) | (3 & (height >> 9));
    b[3] = 0x29;
    cI(c);
  }
}
void GDClass::BitmapSource(uint32_t addr) {
  cI((1UL << 24) | ((addr & 1048575L) << 0));
}
void GDClass::BitmapTransformA(int32_t a) {
  cI((21UL << 24) | ((a & 131071L) << 0));
}
void GDClass::BitmapTransformB(int32_t b) {
  cI((22UL << 24) | ((b & 131071L) << 0));
}
void GDClass::BitmapTransformC(int32_t c) {
  cI((23UL << 24) | ((c & 16777215L) << 0));
}
void GDClass::BitmapTransformD(int32_t d) {
  cI((24UL << 24) | ((d & 131071L) << 0));
}
void GDClass::BitmapTransformE(int32_t e) {
  cI((25UL << 24) | ((e & 131071L) << 0));
}
void GDClass::BitmapTransformF(int32_t f) {
  cI((26UL << 24) | ((f & 16777215L) << 0));
}
void GDClass::BlendFunc(byte src, byte dst) {
  cI((11UL << 24) | ((src & 7L) << 3) | ((dst & 7L) << 0));
}
void GDClass::Call(uint16_t dest) {
  cI((29UL << 24) | ((dest & 2047L) << 0));
}
void GDClass::Cell(byte cell) {
  cI((6UL << 24) | ((cell & 127L) << 0));
}
void GDClass::ClearColorA(byte alpha) {
  cI((15UL << 24) | ((alpha & 255L) << 0));
}
void GDClass::ClearColorRGB(byte red, byte green, byte blue) {
  cI((2UL << 24) | ((red & 255L) << 16) | ((green & 255L) << 8) | ((blue & 255L) << 0));
}
void GDClass::ClearColorRGB(uint32_t rgb) {
  cI((2UL << 24) | (rgb & 0xffffffL));
}
void GDClass::Clear(byte c, byte s, byte t) {
  byte m = (c << 2) | (s << 1) | t;
  cI((38UL << 24) | m);
}
void GDClass::Clear(void) {
  cI((38UL << 24) | 7);
}
void GDClass::ClearStencil(byte s) {
  cI((17UL << 24) | ((s & 255L) << 0));
}
void GDClass::ClearTag(byte s) {
  cI((18UL << 24) | ((s & 255L) << 0));
}
void GDClass::ColorA(byte alpha) {
  cI((16UL << 24) | ((alpha & 255L) << 0));
}
void GDClass::ColorMask(byte r, byte g, byte b, byte a) {
  cI((32UL << 24) | ((r & 1L) << 3) | ((g & 1L) << 2) | ((b & 1L) << 1) | ((a & 1L) << 0));
}
void GDClass::ColorRGB(byte red, byte green, byte blue) {
  // cI((4UL << 24) | ((red & 255L) << 16) | ((green & 255L) << 8) | ((blue & 255L) << 0));
  union {
    uint32_t c;
    uint8_t b[4];
  };
  b[0] = blue;
  b[1] = green;
  b[2] = red;
  b[3] = 4;
  cI(c);
}
void GDClass::ColorRGB(uint32_t rgb) {
  cI((4UL << 24) | (rgb & 0xffffffL));
}
void GDClass::Display(void) {
  cI((0UL << 24));
}
void GDClass::End(void) {
  cI((33UL << 24));
}
void GDClass::Jump(uint16_t dest) {
  cI((30UL << 24) | ((dest & 2047L) << 0));
}
void GDClass::LineWidth(uint16_t width) {
  cI((14UL << 24) | ((width & 4095L) << 0));
}
void GDClass::Macro(byte m) {
  cI((37UL << 24) | ((m & 1L) << 0));
}
void GDClass::PointSize(uint16_t size) {
  cI((13UL << 24) | ((size & 8191L) << 0));
}
void GDClass::RestoreContext(void) {
  cI((35UL << 24));
}
void GDClass::Return(void) {
  cI((36UL << 24));
}
void GDClass::SaveContext(void) {
  cI((34UL << 24));
}
void GDClass::ScissorSize(uint16_t width, uint16_t height) {
  if (ft8xx_model == 0)
    cI((28UL << 24) | ((width & 1023L) << 10) | ((height & 1023L) << 0));
  else
    cI((28UL << 24) | ((width & 4095L) << 12) | ((height & 4095L) << 0));
}
void GDClass::ScissorXY(uint16_t x, uint16_t y) {
  if (ft8xx_model == 0)
    cI((27UL << 24) | ((x & 511L) << 9) | ((y & 511L) << 0));
  else
    cI((27UL << 24) | ((x & 2047L) << 11) | ((y & 2047L) << 0));
}
void GDClass::StencilFunc(byte func, byte ref, byte mask) {
  cI((10UL << 24) | ((func & 7L) << 16) | ((ref & 255L) << 8) | ((mask & 255L) << 0));
}
void GDClass::StencilMask(byte mask) {
  cI((19UL << 24) | ((mask & 255L) << 0));
}
void GDClass::StencilOp(byte sfail, byte spass) {
  cI((12UL << 24) | ((sfail & 7L) << 3) | ((spass & 7L) << 0));
}
void GDClass::TagMask(byte mask) {
  cI((20UL << 24) | ((mask & 1L) << 0));
}
void GDClass::Tag(byte s) {
  cI((3UL << 24) | ((s & 255L) << 0));
}
void GDClass::Vertex2f(int16_t x, int16_t y) {
  // x = int(16 * x);
  // y = int(16 * y);
  cI((1UL << 30) | ((x & 32767L) << 15) | ((y & 32767L) << 0));
}
void GDClass::Vertex2ii(int16_t x, int16_t y, byte handle, byte cell) {

  // In order to draw bitmap on x coordinates higher that 512
  // we use VertexTranslateX which according to documentation:
  // "Specifies the offset added to vertex X coordinates. 
  //  This command allows drawing to be shifted on the screen"
  // Not sure if this is the right approach, but it seems to work

  if (x < 0) {
    // For negative x we shift instead of setting x
    GD.VertexTranslateX(x * 16);
    x = 0; 
  } else if (x >= 512) {
    // for high values we start to use shift
    // and reduces the x value accordingly
    GD.VertexTranslateX(512 * 16);
    x = x - 512;
  } else {
    GD.VertexTranslateX(0);
  }




  
  // cI((2UL << 30) | ((x & 511L) << 21) | ((y & 511L) << 12) | ((handle & 31L) << 7) | ((cell & 127L) << 0));
  union {
    uint32_t c;
    uint8_t b[4];
  };
  b[0] = cell | ((handle & 1) << 7);
  b[1] = (handle >> 1) | (y << 4);
  b[2] = (y >> 4) | (x << 5);
  b[3] = (2 << 6) | (x >> 3);
  cI(c);
}
void GDClass::VertexFormat(byte frac) {
  cI((39UL << 24) | (((frac) & 7) << 0));
}
void GDClass::BitmapLayoutH(byte linestride, byte height) {
  cI((40 << 24) | (((linestride) & 3) << 2) | (((height) & 3) << 0));
}
void GDClass::BitmapSizeH(byte width, byte height) {
  cI((41UL << 24) | (((width) & 3) << 2) | (((height) & 3) << 0));
}
void GDClass::PaletteSource(uint32_t addr) {
  cI((42UL << 24) | (((addr) & 4194303UL) << 0));
}
void GDClass::VertexTranslateX(uint32_t x) {
  cI((43UL << 24) | (((x) & 131071UL) << 0));
}
void GDClass::VertexTranslateY(uint32_t y) {
  cI((44UL << 24) | (((y) & 131071UL) << 0));
}
void GDClass::Nop(void) {
  cI((45UL << 24));
}

void GDClass::cmd_append(uint32_t ptr, uint32_t num) {
  cFFFFFF(0x1e);
  cI(ptr);
  cI(num);
}
void GDClass::cmd_bgcolor(uint32_t c) {
  cFFFFFF(0x09);
  cI(c);
}
void GDClass::cmd_button(int16_t x, int16_t y, uint16_t w, uint16_t h, byte font, uint16_t options, const char *s) {
  cFFFFFF(0x0d);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
  ch(font);
  cH(options);
  cs(s);
}
void GDClass::cmd_calibrate(void) {
  cFFFFFF(0x15);
  cFFFFFF(0xff);
}
void GDClass::cmd_clock(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t h, uint16_t m, uint16_t s, uint16_t ms) {
  cFFFFFF(0x14);
  ch(x);
  ch(y);
  ch(r);
  cH(options);
  cH(h);
  cH(m);
  cH(s);
  cH(ms);
}
void GDClass::cmd_coldstart(void) {
  cFFFFFF(0x32);
}
void GDClass::cmd_dial(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t val) {
  cFFFFFF(0x2d);
  ch(x);
  ch(y);
  ch(r);
  cH(options);
  cH(val);
  cH(0);
}
void GDClass::cmd_dlstart(void) {
  cFFFFFF(0x00);
}
void GDClass::cmd_fgcolor(uint32_t c) {
  cFFFFFF(0x0a);
  cI(c);
}
void GDClass::cmd_gauge(int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range) {
  cFFFFFF(0x13);
  ch(x);
  ch(y);
  ch(r);
  cH(options);
  cH(major);
  cH(minor);
  cH(val);
  cH(range);
}
void GDClass::cmd_getmatrix(void) {
  cFFFFFF(0x33);
  ci(0);
  ci(0);
  ci(0);
  ci(0);
  ci(0);
  ci(0);
}
void GDClass::cmd_getprops(uint32_t &ptr, uint32_t &w, uint32_t &h) {
  cFFFFFF(0x25);
  ptr = GDTR.getwp();
  cI(0);
  w = GDTR.getwp();
  cI(0);
  h = GDTR.getwp();
  cI(0);
}
void GDClass::cmd_getptr(void) {
  cFFFFFF(0x23);
  cI(0);
}
void GDClass::cmd_gradcolor(uint32_t c) {
  cFFFFFF(0x34);
  cI(c);
}
void GDClass::cmd_gradient(int16_t x0, int16_t y0, uint32_t rgb0, int16_t x1, int16_t y1, uint32_t rgb1) {
  cFFFFFF(0x0b);
  ch(x0);
  ch(y0);
  cI(rgb0);
  ch(x1);
  ch(y1);
  cI(rgb1);
}
void GDClass::cmd_inflate(uint32_t ptr) {
  cFFFFFF(0x22);
  cI(ptr);
}
void GDClass::cmd_interrupt(uint32_t ms) {
  cFFFFFF(0x02);
  cI(ms);
}
void GDClass::cmd_keys(int16_t x, int16_t y, int16_t w, int16_t h, byte font, uint16_t options, const char*s) {
  cFFFFFF(0x0e);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
  ch(font);
  cH(options);
  cs(s);
}
void GDClass::cmd_loadidentity(void) {
  cFFFFFF(0x26);
}
void GDClass::cmd_loadimage(uint32_t ptr, int32_t options) {
  cFFFFFF(0x24);
  cI(ptr);
  cI(options);
}
void GDClass::cmd_memcpy(uint32_t dest, uint32_t src, uint32_t num) {
  cFFFFFF(0x1d);
  cI(dest);
  cI(src);
  cI(num);
}
void GDClass::cmd_memset(uint32_t ptr, byte value, uint32_t num) {
  cFFFFFF(0x1b);
  cI(ptr);
  cI((uint32_t)value);
  cI(num);
}
uint32_t GDClass::cmd_memcrc(uint32_t ptr, uint32_t num) {
  cFFFFFF(0x18);
  cI(ptr);
  cI(num);
  uint32_t r = GDTR.getwp();
  cI(0xFFFFFFFF);
  return r;
}
void GDClass::cmd_memwrite(uint32_t ptr, uint32_t num) {
  cFFFFFF(0x1a);
  cI(ptr);
  cI(num);
}
void GDClass::cmd_regwrite(uint32_t ptr, uint32_t val) {
  cFFFFFF(0x1a);
  cI(ptr);
  cI(4UL);
  cI(val);
}
void GDClass::cmd_number(int16_t x, int16_t y, byte font, uint16_t options, uint32_t n) {
  cFFFFFF(0x2e);
  ch(x);
  ch(y);
  ch(font);
  cH(options);
  ci(n);
}
void GDClass::cmd_progress(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range) {
  cFFFFFF(0x0f);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
  cH(options);
  cH(val);
  cH(range);
  cH(0);
}
void GDClass::cmd_regread(uint32_t ptr) {
  cFFFFFF(0x19);
  cI(ptr);
  cI(0);
}
void GDClass::cmd_rotate(int32_t a) {
  cFFFFFF(0x29);
  ci(a);
}
void GDClass::cmd_scale(int32_t sx, int32_t sy) {
  cFFFFFF(0x28);
  ci(sx);
  ci(sy);
}
void GDClass::cmd_screensaver(void) {
  cFFFFFF(0x2f);
}
void GDClass::cmd_scrollbar(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t size, uint16_t range) {
  cFFFFFF(0x11);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
  cH(options);
  cH(val);
  cH(size);
  cH(range);
}
void GDClass::cmd_setfont(byte font, uint32_t ptr) {
  cFFFFFF(0x2b);
  cI(font);
  cI(ptr);
}
void GDClass::cmd_setmatrix(void) {
  cFFFFFF(0x2a);
}
void GDClass::cmd_sketch(int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t ptr, uint16_t format) {
  cFFFFFF(0x30);
  ch(x);
  ch(y);
  cH(w);
  cH(h);
  cI(ptr);
  cI(format);
}
void GDClass::cmd_slider(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t options, uint16_t val, uint16_t range) {
  cFFFFFF(0x10);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
  cH(options);
  cH(val);
  cH(range);
  cH(0);
}
void GDClass::cmd_snapshot(uint32_t ptr) {
  cFFFFFF(0x1f);
  cI(ptr);
}
void GDClass::cmd_spinner(int16_t x, int16_t y, byte style, byte scale) {
  cFFFFFF(0x16);
  ch(x);
  ch(y);
  cH(style);
  cH(scale);
}
void GDClass::cmd_stop(void) {
  cFFFFFF(0x17);
}
void GDClass::cmd_swap(void) {
  cFFFFFF(0x01);
}
void GDClass::cmd_text(int16_t x, int16_t y, byte font, uint16_t options, const char *s) {
  cFFFFFF(0x0c);
  ch(x);
  ch(y);
  ch(font);
  cH(options);
  cs(s);
}
void GDClass::cmd_toggle(int16_t x, int16_t y, int16_t w, byte font, uint16_t options, uint16_t state, const char *s) {
  cFFFFFF(0x12);
  ch(x);
  ch(y);
  ch(w);
  ch(font);
  cH(options);
  cH(state);
  cs(s);
}
void GDClass::cmd_track(int16_t x, int16_t y, uint16_t w, uint16_t h, byte tag) {
  cFFFFFF(0x2c);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
  ch(tag);
  ch(0);
}
void GDClass::cmd_translate(int32_t tx, int32_t ty) {
  cFFFFFF(0x27);
  ci(tx);
  ci(ty);
}
void GDClass::cmd_playvideo(int32_t options) {
  cFFFFFF(0x3a);
  cI(options);
}
void GDClass::cmd_romfont(uint32_t font, uint32_t romslot) {
  cFFFFFF(0x3f);
  cI(font);
  cI(romslot);
}
void GDClass::cmd_mediafifo(uint32_t ptr, uint32_t size) {
  cFFFFFF(0x39);
  cI(ptr);
  cI(size);
}
void GDClass::cmd_setbase(uint32_t b) {
  cFFFFFF(0x38);
  cI(b);
}
void GDClass::cmd_videoframe(uint32_t dst, uint32_t ptr) {
  cFFFFFF(0x41);
  cI(dst);
  cI(ptr);
}
void GDClass::cmd_snapshot2(uint32_t fmt, uint32_t ptr, int16_t x, int16_t y, int16_t w, int16_t h) {
  cFFFFFF(0x37);
  cI(fmt);
  cI(ptr);
  ch(x);
  ch(y);
  ch(w);
  ch(h);
}
void GDClass::cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstchar) {
  cFFFFFF(0x3b);
  cI(font);
  cI(ptr);
  cI(firstchar);
}
void GDClass::cmd_setbitmap(uint32_t source, uint16_t fmt, uint16_t w, uint16_t h) {
  cFFFFFF(0x43);
  cI(source);
  ch(fmt);
  ch(w);
  ch(h);
  ch(0);
}

void GDClass::cmd_setrotate(uint32_t r) {
  cFFFFFF(0x36);
  cI(r);
  // As a special favor, update variables w and h according to this
  // rotation
  w = GDTR.rd16(REG_HSIZE);
  h = GDTR.rd16(REG_VSIZE);
  if (r & 2) {
    int t = h;
    h = w;
    w = t;
  }
}
void GDClass::cmd_videostart() {
  cFFFFFF(0x40);
}

byte GDClass::rd(uint32_t addr) {
  return GDTR.rd(addr);
}
void GDClass::wr(uint32_t addr, uint8_t v) {
  GDTR.wr(addr, v);
}
uint16_t GDClass::rd16(uint32_t addr) {
  return GDTR.rd16(addr);
}
void GDClass::wr16(uint32_t addr, uint16_t v) {
  GDTR.wr16(addr, v);
}
uint32_t GDClass::rd32(uint32_t addr) {
  return GDTR.rd32(addr);
}
void GDClass::wr32(uint32_t addr, uint32_t v) {
  GDTR.wr32(addr, v);
}
//void GDClass::wr_n(uint32_t addr, byte *src, uint32_t n) {
//  GDTR.wr_n(addr, src, n);
//}

void GDClass::cmdbyte(uint8_t b) {
  GDTR.cmdbyte(b);
}
void GDClass::cmd32(uint32_t b) {
  GDTR.cmd32(b);
}
void GDClass::finish(void) {
  GDTR.finish();
}
void GDClass::get_accel(int &x, int &y, int &z) {
  static int f[3];

  for (byte i = 0; i < 3; i++) {
    int a = analogRead(A0 + i);
    int s = (-160 * (a - 376)) >> 6;
    f[i] = ((3 * f[i]) >> 2) + (s >> 2);
  }
  x = f[2];
  y = f[1];
  z = f[0];
}
void GDClass::get_inputs(void) {
  GDTR.finish();
  byte *bi = (byte*)&inputs;
#if defined(DUMPDEV)
  extern FILE* stimfile;
  if (stimfile) {
    byte tag;
    fscanf(stimfile, "%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx",
        &bi[0],
        &bi[1],
        &bi[2],
        &bi[3],
        &bi[4],
        &bi[5],
        &bi[6],
        &bi[7],
        &bi[8],
        &bi[9],
        &bi[10],
        &bi[11],
        &bi[12],
        &bi[13],
        &bi[14],
        &bi[15],
        &bi[16],
        &bi[17]);
    GDTR.wr(REG_TAG, tag);
  } else {
    inputs.x = inputs.y = -32768;
  }
#else
  GDTR.rd_n(bi, REG_TRACKER, 4);
  GDTR.rd_n(bi + 4, REG_TOUCH_RZ, 13);
  GDTR.rd_n(bi + 17, REG_TAG, 1);
  inputs.touching = (inputs.x != -32768);
  inputs.xytouch.set(PIXELS(inputs.x), PIXELS(inputs.y));
#ifdef DUMP_INPUTS
  for (size_t i = 0; i < sizeof(inputs); i++) {
    Serial.print(bi[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
#endif
#endif
}
void GDClass::bulkrd(uint32_t a) {
  GDTR.bulk(a);
}
void GDClass::resume(void) {
  GDTR.resume();
}
void GDClass::__end(void) {
#if !defined(DUMPDEV) && !defined(RASPBERRY_PI)
  GDTR.__end();
#endif
}

void GDClass::reset() {
  GDTR.__end();
  GDTR.wr(REG_CPURESET, 1);
  GDTR.wr(REG_CPURESET, 0);
  GDTR.resume();
}

// Load named file from storage
// returns 0 on failure (e.g. file not found), 1 on success



// Generated by mk_bsod.py. Blue screen with 'ERROR' text
static const PROGMEM uint8_t __bsod[32] = {
0, 255, 255, 255, 96, 0, 0, 2, 7, 0, 0, 38, 12, 255, 255, 255, 240, 0,
90, 0, 31, 0, 0, 6, 69, 82, 82, 79, 82, 0, 0, 0
};
static const PROGMEM uint8_t __bsod_badfile[32] = {
12, 255, 255, 255, 240, 0, 148, 0, 29, 0, 0, 6, 67, 97, 110, 110, 111,
116, 32, 111, 112, 101, 110, 32, 102, 105, 108, 101, 58, 0, 0, 0
};

// Fatal error alert.
// Show a blue screen with message.
// This method never returns.

void GDClass::alert(const char *message)
{
  begin(0);
  copy(__bsod, sizeof(__bsod));
  cmd_text(240, 176, 29, OPT_CENTER, message);
  swap();
  GD.finish();
  for (;;)
    ;
}

void GDClass::safeload(const char *filename)
{
  if (!load(filename)) {
    copy(__bsod, sizeof(__bsod));
    copy(__bsod_badfile, sizeof(__bsod_badfile));
    cmd_text(240, 190, 29, OPT_CENTER, filename);
    swap();
    for (;;)
      ;
  }
}

#define REG_SCREENSHOT_EN    (ft8xx_model ? 0x302010UL : 0x102410UL) // Set to enable screenshot mode
#define REG_SCREENSHOT_Y     (ft8xx_model ? 0x302014UL : 0x102414UL) // Y line register
#define REG_SCREENSHOT_START (ft8xx_model ? 0x302018UL : 0x102418UL) // Screenshot start trigger
#define REG_SCREENSHOT_BUSY  (ft8xx_model ? 0x3020e8UL : 0x1024d8UL) // Screenshot ready flags
#define REG_SCREENSHOT_READ  (ft8xx_model ? 0x302174UL : 0x102554UL) // Set to enable readout
#define RAM_SCREENSHOT       (ft8xx_model ? 0x3c2000UL : 0x1C2000UL) // Screenshot readout buffer

#ifndef DUMPDEV
void GDClass::dumpscreen(void)    
{
  {
    finish();

    wr(REG_SCREENSHOT_EN, 1);
    Serial.write(0xa5);
    Serial.write(GD.w & 0xff);
    Serial.write((GD.w >> 8) & 0xff);
    Serial.write(GD.h & 0xff);
    Serial.write((GD.h >> 8) & 0xff);
    for (int ly = 0; ly < GD.h; ly++) {
      wr16(REG_SCREENSHOT_Y, ly);
      wr(REG_SCREENSHOT_START, 1);
      delay(2);
      while (rd32(REG_SCREENSHOT_BUSY) | rd32(REG_SCREENSHOT_BUSY + 4))
        ;
      wr(REG_SCREENSHOT_READ, 1);
      bulkrd(RAM_SCREENSHOT);
      SPI.transfer(0xff);
      for (int x = 0; x < GD.w; x += 8) {
        union {
          uint32_t v;
          struct {
            uint8_t b, g, r, a;
          };
        } block[8];
        for (int i = 0; i < 8; i++) {
          block[i].b = SPI.transfer(0xff);
          block[i].g = SPI.transfer(0xff);
          block[i].r = SPI.transfer(0xff);
          block[i].a = SPI.transfer(0xff);
        }
        // if (x == 0) block[0].r = 0xff;
        byte difference = 1;
        for (int i = 1, mask = 2; i < 8; i++, mask <<= 1)
          if (block[i].v != block[i-1].v)
            difference |= mask;
        Serial.write(difference);

        for (int i = 0; i < 8; i++)
          if (1 & (difference >> i)) {
            Serial.write(block[i].b);
            Serial.write(block[i].g);
            Serial.write(block[i].r);
          }
      }
      resume();
      wr(REG_SCREENSHOT_READ, 0);
    }
    wr16(REG_SCREENSHOT_EN, 0);
  }
}
#endif

void GDClass::seed(uint16_t n) {
  rseed = n ? n : 7;
}

uint16_t GDClass::random() {
  rseed ^= rseed << 2;
  rseed ^= rseed >> 5;
  rseed ^= rseed << 1;
  return rseed;
}

uint16_t GDClass::random(uint16_t n) {
  uint16_t p = random();
  if (n == (n & -n))
    return p & (n - 1);
  return (uint32_t(p) * n) >> 16;
}

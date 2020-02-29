// ███╗   ███╗ █████╗ ██╗  ██╗    ███████╗██████╗  ██╗ █████╗     ██████╗ ███████╗███╗   ███╗ ██████╗
// ████╗ ████║██╔══██╗╚██╗██╔╝    ╚════██║╚════██╗███║██╔══██╗    ██╔══██╗██╔════╝████╗ ████║██╔═══██╗
// ██╔████╔██║███████║ ╚███╔╝         ██╔╝ █████╔╝╚██║╚██████║    ██║  ██║█████╗  ██╔████╔██║██║   ██║
// ██║╚██╔╝██║██╔══██║ ██╔██╗        ██╔╝ ██╔═══╝  ██║ ╚═══██║    ██║  ██║██╔══╝  ██║╚██╔╝██║██║   ██║
// ██║ ╚═╝ ██║██║  ██║██╔╝ ██╗       ██║  ███████╗ ██║ █████╔╝    ██████╔╝███████╗██║ ╚═╝ ██║╚██████╔╝
// ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝       ╚═╝  ╚══════╝ ╚═╝ ╚════╝     ╚═════╝ ╚══════╝╚═╝     ╚═╝ ╚═════╝

#include <Arduino.h>
// Program to exercise the MD_MAX72XX library
//
// Uses most of the functions in the library
#include <MD_MAX72xx.h>
//#include <SPI.h>

// Turn on debug statements to the serial output
#define DEBUG 1

#if DEBUG
#define PRINT(s, x)     \
  {                     \
    Serial.print(F(s)); \
    Serial.print(x);    \
  }
#define PRINTS(x) Serial.print(F(x))
#define PRINTD(x) Serial.println(x, DEC)
#define PAR \
  LAKLIK(x) \
  mx.control(MD_MAX72XX::INTENSITY, x)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)

#endif

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW //BU ÖNEMLİ
#define MAX_DEVICES 4

#define CLK_PIN 13  // or SCK
#define DATA_PIN 11 // or MOSI
#define CS_PIN 10   // or SS

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// We always wait a bit between updates of the display
#define DELAYTIME 45 // in milliseconds

void rows()
// Demonstrates the use of setRow()
{
  PRINTS("\nRows 0->7");
  mx.clear();

  for (uint8_t row = 0; row < ROW_SIZE; row++)
  {
    mx.control(MD_MAX72XX::INTENSITY, 0);

    mx.setRow(row, random(255));
    delay(2 * DELAYTIME);
    mx.setRow(row, 0x00);
  }
}
void checkboard()
// nested rectangles spanning the entire display
{
  uint8_t chkCols[][2] = {{0x55, 0xaa}, {0x33, 0xcc}, {0x0f, 0xf0}, {0xff, 0x00}};

  PRINTS("\nCheckboard");
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 0);

  for (uint8_t pattern = 0; pattern < sizeof(chkCols) / sizeof(chkCols[0]); pattern++)
  {
    uint8_t col = 0;
    uint8_t idx = 0;
    uint8_t rep = 1 << pattern;

    while (col < mx.getColumnCount())
    {
      for (uint8_t r = 0; r < rep; r++)
        mx.setColumn(col++, chkCols[pattern][idx]); // use odd/even column masks
      idx++;
      if (idx > 1)
        idx = 0;
    }

    delay(1000);
  }
}

void scrollText(const char *p)
{
  uint8_t charWidth;
  uint8_t cBuf[8]; // this should be ok for all built-in fonts

  PRINTS("\nScrolling text");
  mx.clear();
  PARLAKLIK(0);
  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

    for (uint8_t i = 0; i <= charWidth; i++) // allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth)
        mx.setColumn(0, cBuf[i]);
      delay(DELAYTIME);
    }
  }
}

void blinking()
// Uses the test function of the MAX72xx to blink the display on and off.
{
  int nDelay = 1000;

  PRINTS("\nBlinking");
  mx.clear();

  while (nDelay > 0)
  {
    mx.control(MD_MAX72XX::TEST, MD_MAX72XX::ON);
    delay(nDelay);
    mx.control(MD_MAX72XX::TEST, MD_MAX72XX::OFF);
    delay(nDelay);

    nDelay -= DELAYTIME;
  }
}

void showCharset(void)
// Run through display of the the entire font characters set
{
  mx.clear();
  mx.update(MD_MAX72XX::OFF);
  PARLAKLIK(0);
  for (uint16_t i = 0; i < 255; i++)
  {
    mx.clear(0);
    mx.setChar(COL_SIZE - 1, i);

    if (MAX_DEVICES >= 3)
    {
      char hex[3];

      sprintf(hex, "%02X", i);

      mx.clear(1);
      mx.setChar((2 * COL_SIZE) - 1, hex[1]);
      mx.clear(2);
      mx.setChar((3 * COL_SIZE) - 1, hex[0]);
    }

    mx.update();
    delay(10);
  }
  mx.update(MD_MAX72XX::ON);
}

void spiral()
// setPoint() used to draw a spiral across the whole display
{
  PRINTS("\nSpiral in");
  int rmin = 0, rmax = ROW_SIZE - 1;
  int cmin = 0, cmax = (COL_SIZE * MAX_DEVICES) - 1;

  mx.clear();
  while ((rmax > rmin) && (cmax > cmin))
  {
    // do row
    for (int i = cmin; i <= cmax; i++)
    {
      mx.setPoint(rmin, i, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    rmin++;

    // do column
    for (uint8_t i = rmin; i <= rmax; i++)
    {
      mx.setPoint(i, cmax, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    cmax--;

    // do row
    for (int i = cmax; i >= cmin; i--)
    {
      mx.setPoint(rmax, i, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    rmax--;

    // do column
    for (uint8_t i = rmax; i >= rmin; i--)
    {
      mx.setPoint(i, cmin, true);
      delay(DELAYTIME / MAX_DEVICES);
    }
    cmin++;
  }
}
void setup()
{
  mx.begin();

#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MD_MAX72XX Test & Demo]");
  scrollText("MD_MAX72xx Test  ");
}

void loop()
{
  PARLAKLIK(0);
  checkboard();
}

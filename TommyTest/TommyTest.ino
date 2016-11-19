/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341 Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 120
#define PENRADIUS 3
int currentspeed;
bool isrunning;

void setup(void) {
 // while (!Serial);     // used for leonardo debugging
 
  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));
  
  tft.begin();

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  
  
  tft.fillScreen(ILI9341_BLACK);
  
  drawStart();
  drawReset();
  drawIncrement();
  drawDecrement();
  drawSpeed();
  drawCurrentSpeed(currentspeed);
 
  setRunState(true);
}


void loop()
{
  // See if there's any  touch data for us
  if (ts.bufferEmpty()) {
    return;
  }
  /*
  // You can also wait for a touch
  if (! ts.touched()) {
    return;
  }
  */

  // Retrieve a point  
  TS_Point p = ts.getPoint();
  
 /*
  Serial.print("X = "); Serial.print(p.x);
  Serial.print("\tY = "); Serial.print(p.y);
  Serial.print("\tPressure = "); Serial.println(p.z);  
 */
 
  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  
  Serial.print("("); Serial.print(p.x);
  Serial.print(", "); Serial.print(p.y);
  Serial.println(")");
  

  if (p.y < BOXSIZE) {
     setRunState(p.x < BOXSIZE);
  } else if (p.y < 2*BOXSIZE) {
     if (p.x < BOXSIZE) { 
       currentspeed++;
     } else {
       currentspeed--;
     }
     
     drawCurrentSpeed(currentspeed);
  }
  
}

void setRunState(bool runstate) {
  if (isrunning == runstate) return;
  isrunning = runstate;
  if (runstate) {
    tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
    drawReset();
  } else {
    tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
    drawStart();
  }
}

void drawStart() {
    tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.setCursor(0.1*BOXSIZE, 0.4*BOXSIZE);
    tft.println("Start");
}

void drawReset() {
    tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.setCursor(1.1*BOXSIZE, 0.4*BOXSIZE);
    tft.println("Reset");
}

void drawIncrement() {
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(8);
  tft.setCursor(0.35*BOXSIZE, 1.4*BOXSIZE);
  tft.println("+");
}

void drawDecrement() {
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(8);
  tft.setCursor(1.35*BOXSIZE, 1.4*BOXSIZE);
  tft.println("-");
}

void drawSpeed() {
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.setCursor(0.1*BOXSIZE, 2.4*BOXSIZE);
  tft.println("Speed");
}

void drawCurrentSpeed(int currentspeed) {
     tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
     tft.fillRect(BOXSIZE, 2*BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_BLACK);
     tft.setCursor(1.35*BOXSIZE, 2.4*BOXSIZE);
     tft.println(currentspeed);
}

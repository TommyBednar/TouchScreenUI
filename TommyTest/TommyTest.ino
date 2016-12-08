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
#define MIN_SPEED 1
#define MAX_SPEED 50
#define RESET_SPEED 200
#define DELAY_CONSTANT 7200000
int steppin;
int dirpin;
int drivecontrolpin;
int LEDmove;
int LEDreset;
int currentspeed;
int speeddelay;
int currentdistance;
int remainingdistance;
int tempdistance;
bool isrunning = false;
bool stopPressed = false;
int i;

void setup(void) {
 // while (!Serial);     // used for leonardo debugging

  Serial.begin(9600);
  Serial.println(F("Touch Paint!"));

  tft.begin();
  tft.setRotation(2);
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");


  tft.fillScreen(ILI9341_BLACK);
  currentspeed = 5;

  drawStart();
  drawReset();
  drawIncrement();
  drawDecrement();
  drawSpeed();
  drawCurrentSpeed(currentspeed);

  convertSpeed(currentspeed);
  steppin = 22;
  dirpin = 24;
  drivecontrolpin = 26;
  LEDmove = 30;
  LEDreset = 32;
  pinMode(steppin, OUTPUT);
  pinMode(dirpin, OUTPUT);
  pinMode(LEDmove, OUTPUT);
  pinMode(LEDreset, OUTPUT);
  //pinMode(drivecontrolpin, OUTPUT);
  //digitalWrite(drivecontrolpin, LOW);
  digitalWrite(steppin, LOW);
  digitalWrite(dirpin, LOW);
  currentdistance = 125;
  remainingdistance = 275;
  tempdistance = 400;
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

  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  p.x = tft.width() - p.x;
  p.y = tft.height() - p.y;

  if (p.y < BOXSIZE)
  {
     setRunState(p.x < BOXSIZE);
  }
  else if (p.y < 2*BOXSIZE)
  {
     updateSpeed(p.x < BOXSIZE);
     drawCurrentSpeed(currentspeed);
  }

  clearBuffer();
}

void clearBuffer() {
  while (!ts.bufferEmpty())
  {
    ts.getPoint();
  }
}

void updateSpeed(bool increment) {
  if (increment)
  {
    if (currentspeed + 1 > MAX_SPEED)
      return;
    currentspeed++;
  }
  else
  {
    if (currentspeed - 1 < MIN_SPEED)
      return;
    currentspeed--;
  }
}

void setRunState(bool runstate) {

  isrunning = runstate;
  boolean dirneeded = true;
  if (runstate)
  {
    drawStop(true);
    dirneeded = false;
    moveBlock(dirneeded, true);
  }
  else
  {
    drawStop(false);
    dirneeded = true;
    moveBlock(dirneeded, false);
  }
}

void drawStart() {
    tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_GREEN);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.setCursor(0.1*BOXSIZE, 0.4*BOXSIZE);
    tft.println("Start");
}

void drawReset() {
    tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    tft.setCursor(1.1*BOXSIZE, 0.4*BOXSIZE);
    tft.println("Reset");
}

void drawStop(bool pickbox) {
    if(pickbox == false)
      tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
    else
      tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
    tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
    if(pickbox == false)
      tft.setCursor(1.1*BOXSIZE, 0.4*BOXSIZE);
    else
      tft.setCursor(0.1*BOXSIZE, 0.4*BOXSIZE);
    tft.println("Stop");
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
  tft.setCursor(0.1*BOXSIZE, 2.3*BOXSIZE);
  tft.println("Speed");
  tft.setTextSize(2);
  tft.setCursor(.1*BOXSIZE, 2.5*BOXSIZE);
  tft.println("(mm/min)");
}

void drawCurrentSpeed(int currentspeed) {
givendir == false     tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
     tft.fillRect(BOXSIZE, 2*BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_BLACK);
     tft.setCursor(1.35*BOXSIZE, 2.4*BOXSIZE);
     tft.println(currentspeed);
}

void convertSpeed(int givenspeed)
{
    speeddelay = DELAY_CONSTANT / givenspeed;
}

void moveBlock(bool givendir, bool whereStop)
{
    if(!givendir)
    {
       digitalWrite(dirpin, LOW);
       digitalWrite(LEDmove, HIGH);
       convertSpeed(currentspeed);
       currentdistance = 125;
       remainingdistance = 275;
    }
    else
    {
       digitalWrite(dirpin, HIGH);
       digitalWrite(LEDreset, HIGH);
       convertSpeed(RESET_SPEED);  // reset at maximum speed
       //if(stopPressed == true)
         //currentdistance = tempdistance;
       //else
         currentdistance = 400;
    }

    delay(1000);
    clearBuffer();
    //digitalWrite(drivecontrolpin, HIGH);
    for(i = 1; i <= currentdistance; i++)
    {
       if(!ts.bufferEmpty())
       {
         TS_Point p = ts.getPoint();

         // Scale from ~0->4000 to tft.width using the calibration #'s
         p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
         p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

         p.x = tft.width() - p.x;
         p.y = tft.height() - p.y;
         if (p.y < BOXSIZE)
         {
           if(p.x < BOXSIZE && whereStop == true)
           {
             tempdistance = i;
             stopPressed = true;
             break;
           }
           else if(p.x > BOXSIZE && whereStop == false)
           {
             tempdistance = i;
             stopPressed = true;
             break;
           }
         }
         stopPressed = false;
       }

       digitalWrite(steppin, HIGH);
       delay(1);
       digitalWrite(steppin, LOW);
       delayMicroseconds(speeddelay);
    }

    if(givendir == false)
    {
        convertSpeed(RESET_SPEED);
          for(i = 1; i <= remainingdistance; i++)
          {
             if(!ts.bufferEmpty())
             {
               TS_Point p = ts.getPoint();

               // Scale from ~0->4000 to tft.width using the calibration #'s
               p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
               p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

               p.x = tft.width() - p.x;
               p.y = tft.height() - p.y;
               if (p.y < BOXSIZE)
               {
                 if(p.x < BOXSIZE && whereStop == true)
                 {
                   tempdistance = i;
                   stopPressed = true;
                   break;
                 }
                 else if(p.x > BOXSIZE && whereStop == false)
                 {
                    tempdistance = i;
                    stopPressed = true;
                   break;
                 }
               }
               stopPressed = false;
             }
            digitalWrite(steppin, HIGH);
            delay(1);
            digitalWrite(steppin, LOW);
            delayMicroseconds(speeddelay);
         }
    }
    //digitalWrite(drivecontrolpin, LOW);
      drawStart();
      drawReset();
      digitalWrite(LEDmove, LOW);
      digitalWrite(LEDreset, LOW);
}

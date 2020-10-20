#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Adafruit_NeoPixel.h>
#include <BBQ10Keyboard.h>
#include "SDFat.h"
#include "sdios.h"

#ifdef  ARDUINO_FEATHER_ESP32
  #define STMPE_CS 32
  #define TFT_CS 15
  #define TFT_DC 33
  #define SD_CS 14
  #define NEOPIXEL_PIN 27
  #define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(10))
#endif


#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) || \
    defined(ARDUINO_SAMD_ZERO) || defined(__SAMD51__) || defined(__SAM3X8E__) || defined(ARDUINO_NRF52840_FEATHER)
  #define STMPE_CS 6
  #define TFT_CS   9
  #define TFT_DC   10
  #define SD_CS    5
  #define NEOPIXEL_PIN 11
  #define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(50))
#endif


#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

Adafruit_STMPE610 ts(STMPE_CS);
Adafruit_ILI9341 tft(TFT_CS, TFT_DC);
Adafruit_NeoPixel pixels(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
BBQ10Keyboard keyboard;
SdFat  sd;
SdFile root;
SdFile file;

#define MAXFILELENGTH 32
char fileName[MAXFILELENGTH] = {};
void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    yield();  //nrfd52840 native USB delay.  
  }
  Wire.begin();
  // const bool sd = SD.begin(SD_CS);
  
  tft.begin();
  ts.begin();
  
  pixels.begin();
  pixels.setBrightness(30);
  
  keyboard.begin();
  keyboard.setBacklight(0.5f);

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

   // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX);

  tft.print("Hello FeatherWing!\n");
  tft.print("Touch to paint, type to... type\n");

  // List SD card files if available
  // Try at highest speed. If it doesnt work then lower it. 50Mhz seems reasonable.
  if (!sd.begin(SD_CONFIG)) {
    tft.print("SD init error");
    sd.initErrorHalt();
  }
  // Back to top of working dir
  // sd.vwd()->rewind();

  if (root.open("/",O_READ)) {
    tft.print("SD Card contents:\n");
    while (file.openNext(&root, O_READ)) {
      file.printFileSize(&tft);
      tft.print(' ');
      file.printModifyDateTime(&tft);
      tft.print(' ');
      file.printName(&tft);
      tft.println();
      file.close();
    }
    file.close();
  }
  
}

void loop()
{
  // Paint touch
  if (!ts.bufferEmpty()) {
    TS_Point p = ts.getPoint();
    p.x = map(p.x, TS_MINY, TS_MAXY, tft.height(), 0);
    p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
    
    pixels.setPixelColor(0, pixels.Color(255, 0, 255));
    pixels.show(); 
    
    tft.fillCircle(p.y, p.x, 3, ILI9341_MAGENTA);
    
    pixels.clear();
    pixels.show(); 
  }

  // Print keys to screen
  if (keyboard.keyCount()) {
    const BBQ10Keyboard::KeyEvent key = keyboard.keyEvent();
    if (key.state == BBQ10Keyboard::StateRelease) {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show(); 
    
      tft.print(key.key);
      
      pixels.clear();
      pixels.show(); 
    }
  }
}

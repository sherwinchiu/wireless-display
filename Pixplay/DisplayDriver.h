// For TFT Display
#include "Arduino.h"
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

#define TFT_CS         1
#define TFT_RST        3                                            
#define TFT_DC         2
#define TFT_BL         7

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
#define MAX_IMAGE_WIDTH 240
#define MAX_IMAGE_HEIGHT 320
int16_t xpos = 0;
int16_t ypos = 0;

// uint16_t imageBuffer[MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT] PROGMEM;

void setupTFT(){
  Serial.println(F("Hello! ST77xx TFT Test"));
  
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, 200);

  tft.init(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);           // Init ST7789 320x240
  
  tft.fillScreen(ST77XX_BLACK);
  
}



//////////// Tutorial WeatherStat_Screen_h /////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
/////////////////////// end of tutortial WeatherStat_Screen_h ////////////////////////////////////////////////////////////////////////////////////
#ifndef WeatherStat_Screen_h //This goes allways Luke
#define WeatherStat_Screen_h // this also
#include <Arduino.h>      // that one is optional in the .h i think
//including libraries
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_GFX.h"
#include "Adafruit_I2CDevice.h"
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

//Display pins
#define TFT_DC 27
#define TFT_CS 15
#define TFT_RST 4
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_CLK 18

//Touch pins
#define CS_PIN 33
#define TIRQ_PIN  35
#define _sclk 25
#define _mosi 32
#define _miso 39
////////////////////////////////////////////////
struct clima_data{
  String date_for_LCD;
  String time_for_LCD;
  int iaq_for_LCD;
  int temp_for_LCD;
  int co2_for_LCD;
  int gas_for_LCD;
  int pres_for_LCD;
  int humi_for_LCD;
  bool wifiSuccessfulFlag=LOW;
};

struct wifiData{
  char wifiName_for_main[40];
  char wifiPassword_for_main[40];
  bool wifiChangeFlag=LOW;
  bool wifiTimeOut = LOW;
};

struct wifiData1{
  uint8_t wifiNameNo_for_main;
  char wifiPassword_for_main[40];
  bool wifiChangeFlag=LOW;
  bool wifiTimeOut = LOW;
};

////////////////////////////////////////////////////////////////////
void screen_setup();
bool screenHandler(clima_data data_var);
void drawBox(bool leftOrRight, char* boxContent);
void keyOfKeyboard(uint16_t xNumber, uint16_t yNumber, char* upperText, char* lowerText);
char* keyboard(char* inputVar);
wifiData1 wifiSetup(char wifiList[40][40],uint8_t wifiNoT);

///////////////////////////////////////////////////////

//Welcome///////////////////////////////////////////////////////////
//void welcome();

////////////////////////////////////////////////////////////////////
#endif   // that goes allways it closes the library

/////////////////////////////////  end of the library ////////////////////////////////////////////////////////////////

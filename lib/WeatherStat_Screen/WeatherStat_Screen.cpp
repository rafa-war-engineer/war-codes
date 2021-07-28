/**
 * @Author: Rafael Aranda M.Sc Student FH Aachen- CIDESI <rafa>
 * @Date:   2021-27-04
 * @Email:  rafael.aranda@alumni.fh-aachen.de
 * @Filename: WeatherStat_BlynkApp.cpp
 * @Last modified by:   rafa
 * @Last modified time: 2021-27-04
 * @License: CC by-sa
 */

 #include <Arduino.h>                       // tipical arduino bakärei
 #include "WeatherStat_Screen.h"          // name from the library created also knon als Brötchen des Tages
 #include "time.h"                          // time library for some reason

//////////////////////////////////////////////////////////////////////

//tft, init screen
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//ts, init touchscreen
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

void screen_setup();

uint8_t screenCaseVar=0;
uint8_t oldscreenCaseVar=0;

/*
 * cases
 * 0: Welcome
 * 1: Wait without touch
 * 2: Showing data
 * 3: menu case
 *
*/
uint8_t dataShowCaseVar=0;
uint8_t oldDataShowCaseVar=0;
/*
 * cases
 * 0: date and time
 * 1: IAQ
 * 2: Temperature
 * 3: CO2
 * 4: Gas Percentage
 * 5: Pressure
 * 6: Humidity
 * 7: Necessary Actions
 * 8: Waiting for Touch
*/
uint8_t menuCaseVar=0;
uint8_t oldMenuCaseVar=0;
/*
 * 0: "Enter Wifi"
 * 1: "Show Action"
 * 2: Wait for Touch
 * 3: Prepare to enter Wifi name
 * 4: Showing necessary Actions
 * 5: Enter Wifi name
 * 6: Prepare to enter Wifi Password
 * 7: Enter Wifi password
 * 8: Successful?
*/
uint8_t inputCaseVar=0;
uint8_t oldInputCaseVar=0;
/*
 * 0: setting up keyboard
 * 1: capital letters and numbers
 * 2: lowercase letters and symbols
 * 3: decide on key
 * 4: decide on character
*/
uint8_t wifiCaseVar=0;
uint16_t ii=0;
uint16_t xTotalPix=tft.width();         //240
uint16_t yTotalPix=tft.height();        //320
//possible sizes: 1,2,4,5,8,10
uint16_t letterN[6]={40,20,10,8,5,4};   //number of letters that fit into the screen in x and y for different sizes
uint16_t xLetterPix[6];                 //number of pixels per letter in x for different sizes
uint16_t yLetterPix[6];                 //number of pixels per letter in y for different sizes
uint16_t xTouchMax=4095;                //maximum touch value in x
uint16_t yTouchMax=4095;                //maximum touch value in y
bool touchFlag=LOW;                     //flag for the activated touchscreen
bool touchedFlag=LOW;                   //flag for the activated and debounced touchscreen
uint16_t touchX=0;                      //touch position in x in pixels
uint16_t touchY=0;                      //touch position in y in pixels
uint16_t touchXraw=0;                   //raw touch value in x
uint16_t touchYraw=0;                   //raw touch value in y
uint16_t boxEdgeX=30;
uint16_t leftBoxEdgeX=30;               // edge of left box in x
uint16_t rightBoxEdgeX=150;             //edge of right box in x
uint16_t boxEdgeY=232;                  //edge of boxes in y
uint16_t boxX=60;                       //size of boxes in x
uint16_t boxY=32;                       //size of boxes in y
unsigned long oldTime_ms=0;             //help variable for time
unsigned long newTime_ms=0;             //help variable for time
unsigned long oldTimeTouch_ms=0;        //help variable for debouncing time
unsigned long timeChange=0;             //help variable for time
char textContent[120];                  //help variable for printing
uint8_t LEDsize=20;                     //size of LED to show air quality
uint16_t IAQ=0;                         //indoor air quality index
uint16_t temp=0;                        //temperature
uint16_t co2=0;                         //CO2
uint16_t gasPerc=0;                     //Gas Percentage
uint16_t pressure=0;                    //Pressure
uint16_t humidity=0;                    //Humidity
uint16_t iaqColor;                      //Color of air quality LED
char airQuality[21];                    //air quality in words
char iaqAction[120];                    //necessary action
char wifiName[40];                      //name of wifi red
char oldWifiName[40];                   //old name of wifi red
char wifiPassword[40];                  //wifi password
char oldWifiPassword[40];               //old wifi password
uint8_t touchBand=50;                   //touch band width to go left or right in menu
uint16_t keyboardX=70;                  //key width
uint16_t keyboardY=50;                  //key height
uint16_t horKeybLine[5]={700,1400,2100,2800,3500};    //horizontal lines between keyboards
uint16_t verKeybLine[4]={300,1450,2600,3750};         //vertical lines between keyboards
char keyChars[2][12][9];                                  //char on keys in one 3D-matrix
uint8_t j=0;                                          //help variable for indices
uint8_t k=0;                                          //help variable for indices
uint8_t l=0;                                          //help variable for indices
uint8_t m=0;                                          //help variable for indices
char lastDigit[2];                                    //help variable to add onto the output of the keyboard
char helpChar[2];                                     //help Variable for characters
char outputVar[40];                                   //output of the keyboard
bool enteringDoneFlag=LOW;                            //Flag to show that entering of Wifi name and password is done
uint16_t triangleEdge[2];                             //edges of the arrows shown on the keyboard
uint8_t cursorPosX=0;
uint8_t cursorPosY=0;
uint16_t wifiTDistance=375;
uint8_t wifiPageN=0;
uint8_t slotNo=0;
float wifiGap=3.5;
////////////////////////////////////////////////////////////////////////
 //initializing

 void screen_setup(){
 //Setting up SPI communication, the screen and the touchscreen
   SPI.begin(_sclk, _miso, _mosi);
   tft.begin();
   ts.begin();
   ts.setRotation(0);
   tft.setCursor(0,0);
   tft.fillScreen(ILI9341_BLACK);
   tft.setTextColor(ILI9341_WHITE);
   tft.setTextSize(4);
   strcpy(textContent,"Welcome.");
   tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,3*yLetterPix[1]);
   tft.print(textContent);
   delay(5000);

   //calculating xLetterPix and yLetterPix
   for( ii=0;ii<6;ii++){
     xLetterPix[ii]=xTotalPix/letterN[ii];
   }
   for( ii=0;ii<6;ii++){
     yLetterPix[ii]=yTotalPix/letterN[ii];
   }

   //initializing all the characters in the matrix
   strcpy(keyChars[0][0],"1 ");
   strcpy(keyChars[0][1],"2ABC");
   strcpy(keyChars[0][2],"3DEF");
   strcpy(keyChars[0][3],"4GHI");
   strcpy(keyChars[0][4],"5JKL");
   strcpy(keyChars[0][5],"6MNO");
   strcpy(keyChars[0][6],"7PQRS");
   strcpy(keyChars[0][7],"8TUV");
   strcpy(keyChars[0][8],"9WXYZ");
   strcpy(keyChars[0][9],"  ");
   strcpy(keyChars[0][10],"0 ");
   strcpy(keyChars[0][11],"  ");
   strcpy(keyChars[1][0],"- ");
   strcpy(keyChars[1][1],".!?abc");
   strcpy(keyChars[1][2],",:;def");
   strcpy(keyChars[1][3],"/\\_ghi");
   strcpy(keyChars[1][4],"+=^*jkl");
   strcpy(keyChars[1][5],"()[]mno");
   strcpy(keyChars[1][6],"<>{}pqrs");
   strcat(keyChars[1][7],"|'\"tuv");
   strcpy(keyChars[1][8],"&@%wxyz");
   strcpy(keyChars[1][9],"  ");
   strcpy(keyChars[1][10],"~#$ ");
   strcpy(keyChars[1][11],"  ");
 }
 ///////////////////////////

 bool screenHandler(clima_data data_var) {
   bool wifiSetupFlag=LOW;
   struct wifiData my_wifiData;
         //detecting a touch and locating it
         if(ts.tirqTouched()) {
                 if(ts.touched()) {
                         TS_Point p=ts.getPoint();
                         touchFlag=HIGH;
                         touchXraw=p.x;
                         touchYraw=p.y;
                         touchX=p.x*xTotalPix/xTouchMax;
                         touchY=p.y*yTotalPix/yTouchMax;
                 }
         }

         //everything happening on the screen
           yield();
         switch(screenCaseVar) {
         //Welcome
         case 0:
                 tft.fillScreen(ILI9341_BLACK);
                 tft.setTextColor(ILI9341_WHITE);
                 tft.setTextSize(4);
                 strcpy(textContent,"Welcome.");
                 tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,3*yLetterPix[1]);
                 tft.print(textContent);
                 oldscreenCaseVar=screenCaseVar;
                 screenCaseVar=1;
                 timeChange=2000;
                 oldTime_ms=millis();
                 break;
         //Wait without touch
         case 1:
                 if(millis()-oldTime_ms>=timeChange) {
                         switch(oldscreenCaseVar) {
                         case 0:
                                 screenCaseVar=2;
                         case 3:
                                 screenCaseVar=2;
                         }
                 }
                 break;
         //Showing Data
         case 2:
                 switch(dataShowCaseVar) {
                 //date and time
                 case 0:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Date and Time");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(4);
                         tft.setCursor((letterN[2]-data_var.date_for_LCD.length())*xLetterPix[2]/2,5*yLetterPix[1]);
                         tft.print(data_var.date_for_LCD);
                         tft.setTextSize(5);
 //time needs to be added as char array
                         tft.setCursor((letterN[3]-data_var.time_for_LCD.length())*xLetterPix[3]/2,10*yLetterPix[1]);
                         tft.print(data_var.time_for_LCD);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //IAQ
                 case 1:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
 //IAQ needs to be added as integer
                         IAQ=data_var.iaq_for_LCD;
                         strcpy(textContent,"Indoor Air Quality");
                         if(IAQ>=0&&IAQ<=50) {
                                 strcpy(airQuality,"Excellent");
                                 strcpy(iaqAction,"No measures needed");
                                 iaqColor=ILI9341_GREEN;
                         }
                         else if(IAQ>50&&IAQ<=100) {
                                 strcpy(airQuality,"Good");
                                 strcpy(iaqAction,"No measures needed");
                                 iaqColor=ILI9341_GREENYELLOW;
                         }
                         else if(IAQ>100&&IAQ<=150) {
                                 strcpy(airQuality,"Lightly polluted");
                                 strcpy(iaqAction,"Ventilation suggested");
                                 iaqColor=ILI9341_YELLOW;
                         }
                         else if(IAQ>150&&IAQ<=200) {
                                 strcpy(airQuality,"Moderately polluted");
                                 strcpy(iaqAction,"Increase ventilation with clean air");
                                 iaqColor=0xFA00;
                         }
                         else if(IAQ>200&&IAQ<=250) {
                                 strcpy(airQuality,"Heavily polluted");
                                 strcpy(iaqAction,"Optimize ventilation");
                                 iaqColor=ILI9341_RED;
                         }
                         else if(IAQ>250&&IAQ<=350) {
                                 strcpy(airQuality,"Severely polluted");
                                 strcpy(iaqAction,"Contamination should be identified, even without presence of people. Maximize ventilation and reduce attendance.");
                                 iaqColor=ILI9341_PURPLE;
                         }
                         else if(IAQ>350) {
                                 strcpy(airQuality,"Extremely polluted");
                                 strcpy(iaqAction,"Contamination needs to be identified. Avoid presence in room and maximize ventilation.");
                                 iaqColor=ILI9341_MAROON;
                         }
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(5);
                         itoa(IAQ,textContent,10);
                         tft.setCursor((letterN[3]-strlen(textContent))*xLetterPix[3]/2,5*yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(2);
                         strcpy(textContent,airQuality);
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,10*yLetterPix[1]);
                         tft.print(textContent);
                         tft.drawCircle(xTotalPix/2,15*yLetterPix[1],LEDsize+1,ILI9341_WHITE);
                         tft.drawCircle(xTotalPix/2,15*yLetterPix[1],LEDsize,ILI9341_BLACK);
                         tft.fillCircle(xTotalPix/2,15*yLetterPix[1],LEDsize-1,iaqColor);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //temperature
                 case 2:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Temperature");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(5);
 //temperature needs to be added as int
                         temp=data_var.temp_for_LCD;
                         itoa(temp,textContent,10);
                         tft.setCursor((letterN[3]-strlen(textContent))*xLetterPix[3]/2,5*yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(8);
                         tft.setCursor((letterN[4]-2)*xLetterPix[4]/2,10*yLetterPix[1]);
                         tft.print((char)247);
                         tft.print("C");
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //CO2
                 case 3:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"CO2");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(5);
 //CO2 needs to be added as int
                         co2=data_var.co2_for_LCD;
                         itoa(co2,textContent,10);
                         tft.setCursor((letterN[3]-strlen(textContent))*xLetterPix[3]/2,5*yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(8);
                         strcpy(textContent,"ppm");
                         tft.setCursor((letterN[4]-strlen(textContent))*xLetterPix[4]/2,10*yLetterPix[1]);
                         tft.print(textContent);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //gas percentage
                 case 4:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Gas Percentage");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(8);
 //gas percentage needs to be added as int
                         gasPerc=data_var.gas_for_LCD;
                         itoa(gasPerc,textContent,10);
                         strcat(textContent,"%");
                         tft.setCursor((letterN[4]-strlen(textContent))*xLetterPix[4]/2,8*yLetterPix[1]);
                         tft.print(textContent);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //pressure
                 case 5:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Pressure (HPA)");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(8);
 //pressure needs to be added as integer
                         pressure=data_var.pres_for_LCD;
                         itoa(pressure,textContent,10);
                         tft.setCursor((letterN[4]-strlen(textContent))*xLetterPix[4]/2,8*yLetterPix[1]);
                         tft.print(textContent);
                         tft.drawRect(0,15*yLetterPix[1],xTotalPix,yLetterPix[1],ILI9341_WHITE);
                         tft.fillRect(2,15*yLetterPix[1]+2,(double(pressure)-300)/800*(xTotalPix-4),yLetterPix[1]-4,ILI9341_WHITE);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //humidity
                 case 6:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Humidity (%)");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(8);
 //humidity needs to be added as int
                         humidity=data_var.humi_for_LCD;
                         itoa(humidity,textContent,10);
                         tft.setCursor((letterN[4]-strlen(textContent))*xLetterPix[4]/2,8*yLetterPix[1]);
                         tft.print(textContent);
                         tft.drawRect(0,15*yLetterPix[1],xTotalPix,yLetterPix[1],ILI9341_WHITE);
                         tft.fillRect(2,15*yLetterPix[1]+2,double(humidity)/100*(xTotalPix-4),yLetterPix[1]-4,ILI9341_WHITE);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //necessary actions
                 case 7:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Necessary Actions");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(2);
                         strcpy(textContent,iaqAction);
                         tft.setCursor(0,8*yLetterPix[1]);
                         tft.print(textContent);
                         oldDataShowCaseVar=dataShowCaseVar;
                         dataShowCaseVar=8;
                         oldTime_ms=millis();
                         break;
                 //waiting for touch
                 case 8:
                         if(touchFlag==HIGH) {
                                 touchFlag=LOW;
                                 oldTimeTouch_ms=millis();
                                 timeChange=100;
                                 touchedFlag=HIGH;
                         }
                         timeChange=100;
                         if(millis()-oldTimeTouch_ms>=timeChange&&touchedFlag==HIGH) {
                                 dataShowCaseVar=0;
                                 screenCaseVar=3;
                                 touchedFlag=LOW;
                         }
                         timeChange=5000;
                         if(millis()-oldTime_ms>=timeChange) {
                                 dataShowCaseVar=oldDataShowCaseVar+1;
                                 if(dataShowCaseVar>7) {
                                         dataShowCaseVar=0;
                                 }
                         }
                         break;
                 }
                 break;
         //menu case
         //menu is initialized by touching the screen
         //so far provides the option to enter wifi name and password and show the necessary action without switching between all the other data.
         case 3:
                 switch(menuCaseVar) {
                 //Enter Wifi
                 case 0:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(4);
                         strcpy(textContent,"Enter");
                         tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,4*yLetterPix[1]);
                         tft.print(textContent);
                         strcpy(textContent,"WIFI");
                         tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,10*yLetterPix[1]);
                         tft.print(textContent);
                         tft.fillTriangle(0,yTotalPix/2,10,yTotalPix/2+5,10,yTotalPix/2-5,ILI9341_WHITE);
                         tft.fillTriangle(xTotalPix,yTotalPix/2,xTotalPix-10,yTotalPix/2+5,xTotalPix-10,yTotalPix/2-5,ILI9341_WHITE);
                         tft.setTextSize(2);
                         drawBox(0,"BACK");
                         drawBox(1,"OK");
                         oldMenuCaseVar=menuCaseVar;
                         menuCaseVar=2;
                         break;
                 //Show Action
                 case 1:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(4);
                         strcpy(textContent,"Show");
                         tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,4*yLetterPix[1]);
                         tft.print(textContent);
                         strcpy(textContent,"Action");
                         tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,10*yLetterPix[1]);
                         tft.print(textContent);
                         tft.fillTriangle(0,yTotalPix/2,10,yTotalPix/2+5,10,yTotalPix/2-5,ILI9341_WHITE);
                         tft.fillTriangle(xTotalPix,yTotalPix/2,xTotalPix-10,yTotalPix/2+5,xTotalPix-10,yTotalPix/2-5,ILI9341_WHITE);
                         tft.setTextSize(2);
                         drawBox(0,"BACK");
                         drawBox(1,"OK");
                         oldMenuCaseVar=menuCaseVar;
                         menuCaseVar=2;
                         break;
                 //Wait for Touch
                 case 2:
                         if(touchFlag==HIGH) {
                                 touchFlag=LOW;
                                 oldTimeTouch_ms=millis();
                                 timeChange=100;
                                 touchedFlag=HIGH;
                         }
                         //deciding what to do depending on where the touch happenend
                         if(millis()-oldTimeTouch_ms>=timeChange&&touchedFlag==HIGH) {
                                 if(touchX>=leftBoxEdgeX&&touchX<=leftBoxEdgeX+boxX&&touchY>=boxEdgeY&&touchY<=boxEdgeY+boxY) {
                                         switch(oldMenuCaseVar) {
                                         case 0:
                                                 menuCaseVar=0;
                                                 screenCaseVar=2;
                                                 break;
                                         case 1:
                                                 menuCaseVar=0;
                                                 screenCaseVar=2;
                                                 break;
                                         case 4:
                                                 menuCaseVar=1;
                                         }
                                 }
                                 else if(touchX>=rightBoxEdgeX&&touchX<=rightBoxEdgeX+boxX&&touchY>=boxEdgeY&&touchY<=boxEdgeY+boxY) {
                                         switch(oldMenuCaseVar) {
                                         case 0:
                                                 menuCaseVar=3;
                                                 break;
                                         case 1:
                                                 menuCaseVar=4;
                                                 break;
                                         }
                                 }
                                 else if(touchX<touchBand) {
                                         switch(oldMenuCaseVar) {
                                         case 0:
                                                 menuCaseVar=1;
                                                 break;
                                         case 1:
                                                 menuCaseVar=0;
                                                 break;
                                         }
                                 }
                                 else if(touchX>xTotalPix-touchBand) {
                                         switch(oldMenuCaseVar) {
                                         case 0:
                                                 menuCaseVar=1;
                                                 break;
                                         case 1:
                                                 menuCaseVar=0;
                                                 break;
                                         }
                                 }
                                 touchedFlag=LOW;
                                 oldTime_ms=millis();
                         }
                         break;
                 //Wifi setup
                 case 3:
                         wifiSetupFlag=HIGH;
                         break;
                 //Showing Necessary Actions
                 case 4:
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         strcpy(textContent,"Necessary Actions");
                         tft.setCursor((letterN[1]-strlen(textContent))*xLetterPix[1]/2,yLetterPix[1]);
                         tft.print(textContent);
                         tft.setTextSize(2);
                         tft.setCursor(0,8*yLetterPix[1]);
                         tft.print(iaqAction);
                         drawBox(0,"BACK");
                         oldMenuCaseVar=menuCaseVar;
                         menuCaseVar=2;
                         break;
                 //Enter Wifi name
                 case 5:
                         strcpy(wifiName,keyboard(oldWifiName));
                         break;
                 //Prepare to enter Wifi Password
                 case 6:
                         if(enteringDoneFlag) {
                                 enteringDoneFlag=LOW;
                         }
                         tft.fillScreen(ILI9341_BLACK);
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         tft.setCursor(0,0);
                         tft.print("Enter Password:");
                         menuCaseVar=7;
                         inputCaseVar=0;
                         strcpy(oldWifiPassword,wifiPassword);
                         break;
                 //Enter Wifi password
                 case 7:
                         strcpy(wifiPassword,keyboard(oldWifiPassword));
                         newTime_ms=millis();
                         break;
                 //Successful?
                 case 8:
                         my_wifiData.wifiChangeFlag=HIGH;
                         menuCaseVar=11;
                         break;
                 case 9:
                         if(millis()-oldTime_ms>3000) {
                                 inputCaseVar=0;
                                 menuCaseVar=0;
                                 oldTime_ms=millis();
                         }
                         break;
                  case 10:
                         tft.setTextColor(ILI9341_WHITE);
                         tft.setTextSize(2);
                         tft.setCursor(0,0);
                         tft.print("Successful?");
                         tft.setCursor(0,3*xLetterPix[1]);
// message, was entering Wifi successful as string
                         tft.print(textContent);
                         menuCaseVar=9;
                         oldTime_ms=millis();
                         my_wifiData.wifiChangeFlag=LOW;
                         break;
                  case 11:
                         tft.fillScreen(ILI9341_BLACK);
                         if(millis()-newTime_ms>=5000){
                           menuCaseVar=10;
                           strcpy(textContent,"Unsuccessful.");
                         }
                         if(data_var.wifiSuccessfulFlag==HIGH){
                           menuCaseVar=10;
                           strcpy(textContent,"Successful.");
                         }
                         break;
                 }
                 if(millis()-oldTime_ms>=30000) {
                         menuCaseVar=0;
                         inputCaseVar=0;
                         screenCaseVar=2;
                         if(enteringDoneFlag) {
                                 strcpy(oldWifiName,wifiName);
                                 strcpy(oldWifiPassword,wifiPassword);
                         }
                         strcpy(wifiName,oldWifiName);
                         strcpy(wifiPassword,oldWifiPassword);
                 }
                 break;
         }
         strcpy(my_wifiData.wifiName_for_main,wifiName);
         strcpy(my_wifiData.wifiPassword_for_main,wifiPassword);
         return(wifiSetupFlag);
 }


 //Function for drawing OK and BACK boxes
 //0 for left, 1 for right
 void drawBox(bool leftOrRight, char* boxContent){
         if(leftOrRight==LOW) {
                 boxEdgeX=leftBoxEdgeX;
         }
         else {
                 boxEdgeX=rightBoxEdgeX;
         }
         tft.drawRect(boxEdgeX,boxEdgeY,boxX,boxY,ILI9341_WHITE);
         tft.setCursor(boxEdgeX+boxX/2-xLetterPix[1]*strlen(boxContent)/2,boxEdgeY+(boxY-yLetterPix[1])/2);
         tft.println(boxContent);
 }


 //Function for the keyboard
 char* keyboard(char* inputVar){
         switch(inputCaseVar) {
         //setting up keyboard
         case 0:
                 strcpy(outputVar,"");
 //evtl. noch aussortieren.      helpChar[0]=(char)174;
 //evtl. noch aussortieren.      helpChar[1]='\0';
                 tft.setCursor(0,1.5*yLetterPix[1]);
                 inputCaseVar=1;
                 break;
         //setting up keyboard
         //capital letters and numbers
         case 1:
                 tft.fillRect(5,5+4*yLetterPix[1],230,230,ILI9341_BLACK);
                 helpChar[0]=(char)174;
                 helpChar[1]='\0';
                 keyOfKeyboard(1,1,"1",helpChar);
                 keyOfKeyboard(2,1,"2","ABC");
                 keyOfKeyboard(3,1,"3","DEF");
                 keyOfKeyboard(1,2,"4","GHI");
                 keyOfKeyboard(2,2,"5","JKL");
                 keyOfKeyboard(3,2,"6","MNO");
                 keyOfKeyboard(1,3,"7","PQRS");
                 keyOfKeyboard(2,3,"8","TUV");
                 keyOfKeyboard(3,3,"9","WXYZ");
                 triangleEdge[0]=5+keyboardX/2;
                 triangleEdge[1]=41+4*yLetterPix[1]+3*keyboardY;
                 tft.fillTriangle(triangleEdge[0],triangleEdge[1],triangleEdge[0],triangleEdge[1]+yLetterPix[1],triangleEdge[0]-xLetterPix[1],triangleEdge[1]+yLetterPix[1]/2,ILI9341_WHITE);
                 keyOfKeyboard(1,4,"","BACK");
                 keyOfKeyboard(2,4,"0","space");
                 triangleEdge[0]=25+5*keyboardX/2;
                 triangleEdge[1]=41+4*yLetterPix[1]+3*keyboardY;
                 tft.fillTriangle(triangleEdge[0],triangleEdge[1],triangleEdge[0],triangleEdge[1]+yLetterPix[1],triangleEdge[0]+xLetterPix[1],triangleEdge[1]+yLetterPix[1]/2,ILI9341_WHITE);
                 keyOfKeyboard(3,4,"","OK");
                 oldInputCaseVar=inputCaseVar;
                 inputCaseVar=3;
                 break;
         //lowercase letters and symbols
         case 2:
                 tft.fillRect(5,5+4*yLetterPix[1],230,230,ILI9341_BLACK);
                 helpChar[0]=(char)174;
                 helpChar[1]='\0';
                 keyOfKeyboard(1,1,"-",helpChar);
                 keyOfKeyboard(2,1,".!?","abc");
                 keyOfKeyboard(3,1,",:;","def");
                 keyOfKeyboard(1,2,"/\\_","ghi");
                 keyOfKeyboard(2,2,"+=^*","jkl");
                 keyOfKeyboard(3,2,"()[]","mno");
                 keyOfKeyboard(1,3,"<>{}","pqrs");
                 keyOfKeyboard(2,3,"|'\"","tuv");
                 keyOfKeyboard(3,3,"&@%","wxyz");
                 triangleEdge[0]=5+keyboardX/2;
                 triangleEdge[1]=41+4*yLetterPix[1]+3*keyboardY;
                 tft.fillTriangle(triangleEdge[0],triangleEdge[1],triangleEdge[0],triangleEdge[1]+yLetterPix[1],triangleEdge[0]-xLetterPix[1],triangleEdge[1]+yLetterPix[1]/2,ILI9341_WHITE);
                 keyOfKeyboard(1,4,"","BACK");
                 keyOfKeyboard(2,4,"~#$","space");
                 triangleEdge[0]=25+5*keyboardX/2;
                 triangleEdge[1]=41+4*yLetterPix[1]+3*keyboardY;
                 tft.fillTriangle(triangleEdge[0],triangleEdge[1],triangleEdge[0],triangleEdge[1]+yLetterPix[1],triangleEdge[0]+xLetterPix[1],triangleEdge[1]+yLetterPix[1]/2,ILI9341_WHITE);
                 keyOfKeyboard(3,4,"","OK");
                 oldInputCaseVar=inputCaseVar;
                 inputCaseVar=3;
                 break;
         //decide on key
         case 3:
                 if(touchFlag==HIGH) {
                         touchFlag=LOW;
                         oldTimeTouch_ms=millis();
                         touchedFlag=HIGH;
                 }
                 timeChange=100;
                 if(millis()-oldTimeTouch_ms>=timeChange&&touchedFlag==HIGH) {
                         for (ii=0; ii<3; ii++) {
                                 for (j=0; j<4; j++) {
                                         if(touchXraw>verKeybLine[ii]&&touchXraw<verKeybLine[ii+1]&&touchYraw>horKeybLine[j]&&touchYraw<horKeybLine[j+1]) {
                                                 k=ii;
                                                 l=j;
                                         }
                                 }
                         }
                         touchedFlag=LOW;
                         inputCaseVar=4;
                         //Number of key
                         ii=3*l+k;
                         j=0;
                         oldTime_ms=millis();
                 }
                 break;
         //decide on character
         case 4:
                 if(touchFlag==HIGH) {
                         touchFlag=LOW;
                         oldTimeTouch_ms=millis();
                         touchedFlag=HIGH;
                 }
                 timeChange=100;
                 if(millis()-oldTimeTouch_ms>=timeChange&&touchedFlag==HIGH) {
                         if(touchXraw>verKeybLine[k]&&touchXraw<verKeybLine[k+1]&&touchYraw>horKeybLine[l]&&touchYraw<horKeybLine[l+1]) {
                                 j++;
                                 if(j>=strlen(keyChars[oldInputCaseVar-1][ii])) {
                                         j=0;
                                 }
                                 if(ii==0&&j==1) {
                                         tft.fillRect((strlen(outputVar)-1)*xLetterPix[1],1.5*yLetterPix[1],2*xLetterPix[1],yLetterPix[1],ILI9341_BLACK);
                                         outputVar[strlen(outputVar)-1]='\0';
                                 }
                                 else if(ii==9&&j==1) {
                                         strcpy(outputVar,inputVar);
                                         inputCaseVar=0;
                                         menuCaseVar=0;
                                         wifiCaseVar=0;
                                 }
                                 else if(ii==11&&j==1) {
                                         inputCaseVar=0;
                                         enteringDoneFlag=HIGH;
                                         wifiCaseVar=5;
                                         if(menuCaseVar==5) {
                                                 menuCaseVar=6;
                                         }
                                         else {
                                                 menuCaseVar=8;
                                         }
                                 }
                                 else{
                                         tft.fillRect(strlen(outputVar)*xLetterPix[1],1.5*yLetterPix[1],2*xLetterPix[1],yLetterPix[1],ILI9341_BLACK);
                                         oldTime_ms=millis();
                                 }
                         }
                         touchedFlag=LOW;
                 }
                 timeChange=1000;
                 if(millis()-oldTime_ms>=timeChange) {
                         inputCaseVar=3;
                         if(strlen(outputVar)>20) {
                                 tft.fillRect((-19+strlen(outputVar))*xLetterPix[1],2.5*yLetterPix[1],2*xLetterPix[1],yLetterPix[1],ILI9341_BLACK);
                                 tft.setCursor((strlen(outputVar)-20)*xLetterPix[1],2.5*yLetterPix[1]);
                         }
                         else {
                                 tft.fillRect((1+strlen(outputVar))*xLetterPix[1],1.5*yLetterPix[1],2*xLetterPix[1],yLetterPix[1],ILI9341_BLACK);
                                 tft.setCursor(strlen(outputVar)*xLetterPix[1],1.5*yLetterPix[1]);
                         }
                         if ((ii==9||ii==11)&&j==0) {
                                 inputCaseVar=!(oldInputCaseVar-1)+1;
                         }
                         else if (!(ii==0&&j==1)&&!(ii==9)&&!(ii==11)) {
                                 lastDigit[0]=keyChars[oldInputCaseVar-1][ii][j];
                                 lastDigit[1]='\0';
                                 strcat(outputVar,lastDigit);
                                 tft.print(lastDigit);
                         }

                 }
                 break;
         }
         return outputVar;
 }


 //Function for drawing a particular key on the screen
 /*
  * xNumber: number of key in x
  * yNumber: number of key in y
  */
 void keyOfKeyboard(uint16_t xNumber, uint16_t yNumber, char* upperText, char* lowerText){
         xNumber--;
         yNumber--;
         tft.drawRoundRect(5+xNumber*(10+keyboardX),5+4*yLetterPix[1]+yNumber*(10+keyboardY),keyboardX,keyboardY,4,ILI9341_WHITE);
         tft.setCursor(5+keyboardX/2+xNumber*(10+keyboardX)-strlen(upperText)*xLetterPix[1]/2,11+4*yLetterPix[1]+yNumber*(10+keyboardY));
         tft.print(upperText);
         tft.setCursor(5+keyboardX/2+xNumber*(10+keyboardX)-strlen(lowerText)*xLetterPix[1]/2,16+5*yLetterPix[1]+yNumber*(10+keyboardY));
         tft.print(lowerText);
 }


 wifiData1 wifiSetup(char wifiList[40][40], uint8_t wifiNoT){
   wifiData1 wifiData_to_main;
   //detecting a touch and locating it
   if(ts.tirqTouched()) {
           if(ts.touched()) {
                   TS_Point p=ts.getPoint();
                   touchFlag=HIGH;
                   touchXraw=p.x;
                   touchYraw=p.y;
           }
   }
   switch(wifiCaseVar){
     case 0:
        Serial.print("menuCaseVar: ");
        Serial.println(menuCaseVar);
        wifiData_to_main.wifiNameNo_for_main=0;
        wifiData_to_main.wifiChangeFlag=LOW;
        k=0;
        slotNo=0;
        l=wifiNoT+1;
        wifiPageN=0;
        while(l>8){
          l=l-8;
          wifiPageN++;
        }
        wifiCaseVar=1;
        break;
     case 1:
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.setCursor(0,0);
        tft.print("Choose Wifi:");
        if(k==wifiPageN){
           l=wifiNoT+1-wifiPageN*8;
        }
        else{
           l=8;
        }
        for(j=0;j<l;j++){
            if(strlen(wifiList[j]+k*8)>20){
              wifiGap=2;
            }
            else{
              wifiGap=2.5;
            }
            tft.setCursor(0,(2*j+wifiGap)*yLetterPix[1]);
            tft.print(wifiList[j+k*8]);
        }
        if(wifiNoT>=8){
            tft.fillTriangle(xTotalPix/2-5,18.5*yLetterPix[1],xTotalPix/2+5,18.5*yLetterPix[1],xTotalPix/2,18.5*yLetterPix[1]+10,ILI9341_WHITE);
        }
        wifiCaseVar=2;
        oldTime_ms=millis();
        break;
     case 2:
        if(touchFlag==HIGH) {
             touchFlag=LOW;
             oldTimeTouch_ms=millis();
             oldTime_ms=millis();
             touchedFlag=HIGH;
        }
        timeChange=100;
        if(millis()-oldTimeTouch_ms>=timeChange&&touchedFlag==HIGH) {
             touchY=touchYraw;
             slotNo=0;
             while(touchY>wifiTDistance){
                  touchY=touchY-wifiTDistance;
                  slotNo++;
             }
             Serial.println(wifiList[slotNo+wifiPageN*8]);
             Serial.print("SlotNo: ");
             Serial.println(slotNo);
             Serial.print("Page: ");
             Serial.println(wifiPageN);
             Serial.print("k: ");
             Serial.println(k);
             touchedFlag=LOW;
        }
        if(slotNo>8){
             k++;
             if(k>wifiPageN){
               k=0;
             }
             wifiCaseVar=1;
        }
        if(slotNo+k*8>wifiNoT){
          slotNo=0;
        }
        if(slotNo==0){
             wifiData_to_main.wifiNameNo_for_main=wifiNoT+1;
        }
        if(slotNo>0) {
             wifiData_to_main.wifiNameNo_for_main=slotNo-1+k*8;
        }
        if(slotNo>0&&wifiData_to_main.wifiNameNo_for_main<wifiNoT+1){
             wifiCaseVar=3;
        }
        else if (millis()-oldTime_ms>30000||wifiData_to_main.wifiNameNo_for_main==wifiNoT+1){
             wifiCaseVar=6;
        }
        break;
     case 3:
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.setCursor(0,0);
        tft.print("Enter Password:");
        wifiCaseVar=4;
        inputCaseVar=0;
        strcpy(oldWifiPassword,wifiPassword);
        break;
     case 4:
        strcpy(wifiPassword,keyboard(oldWifiPassword));
        newTime_ms=millis();
        break;
     case 5:
        strcpy(wifiData_to_main.wifiPassword_for_main,wifiPassword);
        wifiData_to_main.wifiChangeFlag=HIGH;
        wifiCaseVar=0;
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.setCursor(0,0);
        tft.print("Verifying");
        wifiCaseVar=0;
        if(menuCaseVar==3){
          menuCaseVar=11;
          Serial.println("MenuCaseVar=11");
        }
        Serial.print("menuCaseVar: ");
        Serial.println(menuCaseVar);
        break;
     case 6:
        strcpy(wifiData_to_main.wifiPassword_for_main,"");
        wifiData_to_main.wifiChangeFlag=HIGH;
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(2);
        tft.setCursor(0,0);
        tft.print("Verifying.");
        wifiCaseVar=0;
        if(menuCaseVar==3){
          menuCaseVar=11;
          Serial.println("MenuCaseVar=11");
        }
        Serial.print("menuCaseVar: ");
        Serial.println(menuCaseVar);
        break;
   }
   return(wifiData_to_main);
 }

 //Welcome///////////////////////////////////////////////////////////
 /*void welcome(){
   tft.fillScreen(ILI9341_BLACK);
     tft.setTextColor(ILI9341_WHITE);
     tft.setTextSize(4);
     strcpy(textContent,"Welcome.");
     tft.setCursor((letterN[2]-strlen(textContent))*xLetterPix[2]/2,3*yLetterPix[1]);
     tft.print(textContent);
 }*/
 ////////////////////////////////////////////////////////////////////

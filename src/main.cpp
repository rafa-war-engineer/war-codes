/**
 * @Author: Daniel Murrieta M.Sc. Student FH Aachen - CIDESI <daniel>
 * @Date:   2021-01-03T00:56:38+01:00
 * @Email:  daniel.murrieta-alvarez@alumni.fh-aachen.de
 * @Blynk_App_Asistent: Rafael Aranda M.Sc Student FH Aachen- CIDESI <rafa>
 * @Date:   2021-27-04
 * @Email:  rafael.aranda@alumni.fh-aachen.de
 * @Filename: main.cpp
 * @Last modified by:   daniel
 * @Last modified time: 2021-05-17T12:41:25+02:00
 * @License: CC by-sa
 *
 */
/////////////################# headers #####################////////////////

//upload_port = /dev/cu.SLAB_USBtoUART	//Danys pc
#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "bsec.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include "WebServer.h"
#include "time.h"
#include <stdlib.h> //For future use in weather forecasting
#include <stdio.h> //For future use in weather forecasting
#include <string>
#include <BlynkSimpleEsp32.h>
#include "WeatherStat_NTP.h"
#include "WeatherStat_BlynkApp.h"
#include "WeatherStat_CO2sensor.h"
#include "WeatherStat_Messages.h"
#include "WeatherStat_Screen.h"
#include "WeatherStat_Memory.h"
/////////////################# directives #####################////////////////
#define Number_susc_sens 11
#define STATE_SAVE_PERIOD UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day
#define DEBUG
#define MAX_NUMBER_NETWORKS 10
/////////////################# variables #####################////////////////
const byte led_gpio = 13; // the PWM pin the LED is attached to
int PWMchannel = 0;
int i = 0;
char flag=0;
uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};
uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t stateUpdateCounter = 0;
String output;
float varTemp, varPres, varHumi, varGasR, varCo2E, varBVoc, varGasP,varSiaq,varIaq;
int varIaqAcc, varStab, varRunI,   varSiaqAcc, varCo2eAcc;
int varBVocAcc, varGasPAcc;
float varCo2M=400.0;
String cadena_envio,dateString,start_time,fechaString;
volatile int segundos=0;
uint8_t number_net=0;
char wifiNameList[40][40];
/////////////################# functions #####################////////////////
void loop4(void *parameter);
void loop3(void *parameter);
void loop2(void *parameter);
void loop1(void *parameter);
void checkIaqSensorStatus(void);
void loadState(void);
void updateState(void);
void myTimerEvent(void);
void handleRoot();
bool loadFromSpiffs(String path);
void handleWebRequests();
void dataSensorRequest(void);
void JsonStringFormat(void);
void LCD_vars();
void IRAM_ATTR isr();
/////////////################# handlers definition #####################///////
/////////////################# Web Specific #####################///////
//char *ssid = "FRITZ!Box 6591 Cable SW";         // Daniel
//char *password = "62407078731195560963";
//String password = "62407078731195560963";
//char passw2[30];
// char *ssid = "FRITZ!Box 6591 Cable BE";          // replace with your SSID
// char *password = "07225443701792235194";  // replace with your Password
//char *ssid = "Vodafone-FC6F = FRITZ!Box 6591 Cable BE";          // replace with your SSID/char *password = "07225443701792235194";  // replace with your Password";          // replace with your SSID
//char *password = "pW6298625330626571";  //
WiFiClient my_client;
WebServer server(80);
/////////////################# NTP Client Specific #####################///////
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;  //Germany is GMT +1, expressed in seconds
const int daylightOffset_sec = 3600;   // There is one extra hour in DST
/////////////################# Blynk  config ###################///////
char auth[] = "DZJ5YStEBLYrOid9YuJH-Qm3QDPuy-Oe";    // token from the app
BlynkTimer timer;
// data request on app time
/////////////################# Blynk  variables ###################///////
iaq_level property; // define the structure property for iaq level of the app
/////////////################# CO2_sensor  config ###################///////
volatile long current_time=0;
volatile long StartLow = 0;
volatile long StartHigh = 0;
volatile long LowLevel=0;
volatile long HighLevel=0;
volatile bool NEW_DATA = false;
volatile bool FIRST_TIME = false;
/////////////################# Touch_LCD ###################///////
clima_data var_data;
bool wifiFlag=LOW;
bool settingWifi=LOW;
bool settingWifiOld=LOW;
wifiData wifiData_in_main;
wifiData1 wifiData_in_main1;
/////////////################# Arduino sh%& #####################///////////////
TaskHandle_t Task4;
TaskHandle_t Task3;
TaskHandle_t Task2;
TaskHandle_t Task1;
Bsec iaqSensor;
enum NetCAses {NO_NETWORK,NET_FROM_MEM,NET_FROM_USR};
NetCAses current_case;
/////////////################# SETUP #####################////////////////
void setup() {
  //////// Screen initialization
        char *temporalssid2;//defina una apuntador vacio de una cadena
        char *temporalpw2;

        Serial.begin(115200);
        Wire.begin();
        EEPROM.begin(512);
        Serial.println("--------------------------------------");
        Serial.println(String(ESP.getChipModel())+" "+String(ESP.getChipCores())+" Cores Running @ "+String(ESP.getCpuFreqMHz())+"MHz");
///SPIFSSS begin and check
        if(!SPIFFS.begin()) {
                Serial.println("An Error has occurred while mounting SPIFFS");
                return;
        }
        else Serial.println("SPIFFS Mounted correctly");

////// Wifi Scannig networks
        number_net = WiFi.scanNetworks();
        if (number_net>39){
          number_net=39;
        }
        Serial.println(number_net);
        for (int j=0; j<number_net; j++)//Print Networks
        {
                Serial.print(j + 1);
                Serial.print(": ");
                Serial.println(WiFi.SSID(j));
                char temp[30];
                String tempStr=WiFi.SSID(j);
                tempStr.toCharArray(temp,(tempStr.length()+1));
                strcpy(wifiNameList[j],temp);
        }
        Serial.println(String(number_net+1)+": Usage with no network");//quitar
        strcpy(wifiNameList[number_net],"Use without network");//quitar

//Wifi selection in screen

        Serial.println("Selected option: ");
        int selected_network=200;
        String number;
        /*Network select*/
        /*do {
                while(Serial.available()!=0)
                {
                        char recep = Serial.read();
                        number = number + recep;
                        if (recep==13) {
                                selected_network = number.toInt();
                                number="";
                        }
                }
        } while(selected_network > (number_net+1) || selected_network<0);*/
        screen_setup();
        wifiData_in_main1.wifiChangeFlag=LOW;
        while(wifiFlag==LOW){
                wifiData_in_main1=wifiSetup(wifiNameList,number_net);
                wifiFlag=wifiData_in_main1.wifiChangeFlag;
        }
        wifiFlag=LOW;
        selected_network=wifiData_in_main1.wifiNameNo_for_main;
        Serial.println(selected_network);
        if(selected_network <= number_net) {
//                String intercambio = WiFi.SSID(selected_network-1);//Asigna el nombre de la red seleccionada a variable
//                temporalssid2 = strcpy(temporalssid2,intercambio);//convierte cadena Ardunio a cadena cpp
                int wifiNo=wifiData_in_main1.wifiNameNo_for_main;
                temporalssid2=wifiNameList[wifiNo];
                Serial.println(temporalssid2);
                /*Passwor reception*/
/*                bool Pass_received = false;
                String passtempo="";
                do {
                        while(Serial.available()!=0)
                        {
                                char recep = Serial.read();
                                if(int(recep)!=10)
                                {
                                        if (int(recep)==13) Pass_received=true;
                                        else passtempo += recep;
                                }
                        }
                } while(!Pass_received);*/

                temporalpw2 = wifiData_in_main1.wifiPassword_for_main;
                //strcpy(temporalpw2,wifiData_in_main1.wifiPassword_for_main);
////// Wifi Configs
                Serial.println(temporalssid2);
                Serial.println("Password:");
                Serial.println(temporalpw2);
                WiFi.begin(temporalssid2, temporalpw2);
                //WiFi.begin(ssid, password);
                int k = 0;
                while (WiFi.status() != WL_CONNECTED)
                {
                        delay(1000);
                        Serial.println("Connecting to WiFi...");
                        k++;
                        if(k == 10) ESP.restart();
                }

                Serial.print("\nConnected to the WiFi network: ");
                Serial.println(WiFi.SSID());
                Serial.print("IP address:");
                Serial.println(WiFi.localIP());
        }
        else{
                Serial.println("Offline mode generating own network with IP:");
                WiFi.mode(WIFI_MODE_APSTA);
                String name="WheatherStat_AP";
                String passwort = "123456789";
                strcpy(temporalssid2,name);
                strcpy(temporalpw2,passwort);
                WiFi.softAP("WheatherStat_AP",NULL,1,0,4);
//WiFi.softAP(const char *ssid, optional const char *passphrase = NULL, optional int channel = 1, optional int ssid_hidden = 0, optional int max_connection = 4)
//WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
                Serial.println(WiFi.softAPIP());
                dateString="Time na.";
                fechaString = "Date na.";
        }
////// Blynk begin config

       //Blynk.begin(auth, temporalssid2, temporalpw2);
      //  free(temporalssid2);
        //free(temporalpw2);
        //timer.setInterval(1000L, myTimerEvent);//Verificar esto con la app
/////Initialize BME680 Sensor
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
/////Initialize BME680 Sensor
        iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
        output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
        Serial.println(output);
        checkIaqSensorStatus();
        iaqSensor.setState(bsec_config_iaq);
        checkIaqSensorStatus();
        loadState();

        bsec_virtual_sensor_t sensorList[Number_susc_sens] = {
                //BSEC_OUTPUT_RAW_TEMPERATURE,//Temperature directly measured by BME680 in degree Celsius.
                BSEC_OUTPUT_RAW_PRESSURE,//Pressure directly measured by the BME680 in Pa.
                //BSEC_OUTPUT_RAW_HUMIDITY,//Relative humidity directly measured by the BME680 in %.
                BSEC_OUTPUT_RAW_GAS,//Gas resistance measured directly by the BME680 in Ohm.The resistance value changes due to varying VOC concentrations
                BSEC_OUTPUT_IAQ,//During operation, algorithms automatically calibrate and adapt themselves to the typical environments where the sensor is operated (e.g., home, workplace, inside a car, etc.).This automatic background calibration ensures that users experience consistent IAQ performance. The calibration process considers the recent measurement history
                BSEC_OUTPUT_STATIC_IAQ,/*!<“Static” Index for Air Quality, especially recommended for stationary devices (w/ o auto-trimming algorithm) */
                BSEC_OUTPUT_CO2_EQUIVALENT,//Estimation of the CO2 level in ppm.
                BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,//Conversion into breath-VOC equivalents in ppm concentration.
                BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,//Temperature which is compensated for internal cross- influences caused by the BME sensor
                BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,//Relative humidity which is compensated for internal cross- influences caused by the BME sensor
                BSEC_OUTPUT_GAS_PERCENTAGE,/*Alternative indicator for air pollution which rates the current raw gas sensor resistance value based on the individual sensor history:*/
                BSEC_OUTPUT_STABILIZATION_STATUS,//Indicates if the sensor is undergoing initial stabilization during its first use after production
                BSEC_OUTPUT_RUN_IN_STATUS//Indicates when the sensor is ready after switch-on
        };
        iaqSensor.updateSubscription(sensorList, Number_susc_sens, BSEC_SAMPLE_RATE_LP);
        checkIaqSensorStatus();
/////Task configuration FreeRTOS
        xTaskCreatePinnedToCore(loop1,"Task_1",15000,NULL,1,&Task1,1);
        delay(500);
        xTaskCreatePinnedToCore(loop2,"Task_2",15000,NULL,1,&Task2,0);
        delay(500);
        xTaskCreatePinnedToCore(loop3,"Task_3",20000,NULL,1,&Task3,0);
        delay(500);
        xTaskCreatePinnedToCore(loop4,"Task_4",15000,NULL,1,&Task4,0);
        delay(500);

/////Web Handlers
        server.on("/", handleRoot);
        server.on("/info", [](){
                server.send(200, "application/json", cadena_envio);
        });
        server.on("/temperature", HTTP_GET,[](){
                server.send(200, "text/json", String(varTemp));
        });
        server.on("/iaq", HTTP_GET,[](){
                server.send(200, "text/json", String(varIaq));
        });
        server.on("/co2", HTTP_GET,[](){
                server.send(200, "text/json", String(varCo2M));
        });
        server.on("/breathvoc_h", HTTP_GET,[](){
                server.send(200, "text/json", String(varBVoc));
        });
/////Initialize Server
        server.onNotFound(handleWebRequests);
        server.begin();// begin server
/////Define starting time
        if(WiFi.status() == WL_CONNECTED)
        start_time = getDatum(IN_LETTERS) +" "+getZeit();
        #ifdef DEBUG
        if(WiFi.status() == WL_CONNECTED)
        Serial.println(start_time);
        #endif
        ////// Config for readding CO2_sensor
        pinMode(MHZ19_PWM_PIN, INPUT);        //MHZ19 PWM Pin als Eingang konfigurieren
        attachInterrupt(MHZ19_PWM_PIN, isr, CHANGE);
}
/////////////################# LOOP Executed in Core 1 #####################////////////////
void loop(){
        vTaskDelay(1);
}
/////////////################# Touchscreen LCD management #####################////////////////
void loop1(void *parameter) {
        //call Screen handle

        //Serial.print("---------### LOOP1");
        //Serial.println(i);//
        //yield();
        while(1) {
                //hw_wdt_disable();
                //Serial.println(" -----in loop 1");
                //yield();
                LCD_vars();
                //Serial.println("  -Outside LCD_vars()");
                //yield();
                settingWifiOld=settingWifi;
                settingWifi=screenHandler(var_data);
                if(settingWifi==HIGH){
                        if(settingWifiOld==LOW){
                          ////// Wifi Scannig networks
                                  number_net = WiFi.scanNetworks();
                                  if (number_net>39){
                                    number_net=39;
                                  }
                                  Serial.println(number_net);
                                  for (int j=0; j<number_net; j++)//Print Networks
                                  {
                                          Serial.print(j + 1);
                                          Serial.print(": ");
                                          Serial.println(WiFi.SSID(j));
                                          char temp[30];
                                          String tempStr=WiFi.SSID(j);
                                          tempStr.toCharArray(temp,(tempStr.length()+1));
                                          strcpy(wifiNameList[j],temp);
                                  }
                                  Serial.println(String(number_net+1)+": Usage with no network");//quitar
                                  strcpy(wifiNameList[number_net],"Use without network");//quitar
                        }
                        wifiData_in_main1=wifiSetup(wifiNameList,number_net);
                        wifiFlag=wifiData_in_main1.wifiChangeFlag;
                }
                //  Serial.println("      -Outside screenHandler()");
//                strcpy(ssid,wifiData_in_main.wifiName_for_main);
//                strcpy(password,wifiData_in_main.wifiPassword_for_main);+
                //Serial.println(wifiData_in_main.wifiChangeFlag);
                if(wifiFlag==HIGH) {
                        char *temporalssid2;//defina una apuntador vacio de una cadena
                        char *temporalpw2;

                        //Serial.println("          -inside   if(wifiData_in_main.wifiChangeFlag");
                        temporalssid2=wifiNameList[wifiData_in_main1.wifiNameNo_for_main];

                        temporalpw2 = wifiData_in_main1.wifiPassword_for_main;
                        //strcpy(temporalpw2,wifiData_in_main1.wifiPassword_for_main);
        ////// Wifi Configs
                        WiFi.begin(temporalssid2, temporalpw2);
                        char j=0;
                        while ((WiFi.status() != WL_CONNECTED)&&j<4)
                        {
                                delay(1000);
                                Serial.println("Connecting to WiFi..."+String(j));
                                j++;
                        }
                        if(j<4) {
                                var_data.wifiSuccessfulFlag=HIGH;
                        }
                        else{
                                var_data.wifiSuccessfulFlag=LOW;
                        }
                }
                else{
                        var_data.wifiSuccessfulFlag=LOW;
                }
                vTaskDelay(1);
        }//////// cierra while
}
/////////////################# LOOP Executed in Core 0 #####################////////////////
void loop2(void *parameter) {

        while(true) {
                if (iaqSensor.run()) {   // If new data is available
                        dataSensorRequest();
                        JsonStringFormat();
                        updateState();
                        //Serial.println("- Loop2 - BME680");
                } else {
                        checkIaqSensorStatus();
                }
                server.handleClient();/// NO QUITAR!!!
                //vTaskDelay(1);
        }///////// main while loop
}
/////////////################# LOOP3 CO2 Task #####################////////////////
void loop3(void *parameter) {
        while(true) {
                if(NEW_DATA) {
                        //Serial.println("- Loop3 - CO2 calc");
                        //Serial.print("L->"+String(LowLevel));
                        varCo2M = get_CO2_measure(HighLevel,LowLevel);
                        //Serial.println(" H->"+String(HighLevel));
                        NEW_DATA = false;
                }
                vTaskDelay(1);
        }
/////////////################# funciton  #####################////////////////
}
/////////////################# LOOP4 Blynk app Task #####################////////////////
void loop4(void *parameter) {
        while(true) {
              if(WiFi.status()==WL_CONNECTED){
                Blynk.run();  // run code of the app
                //timer.run(); // establishes comunication to the app in a time interval to load sensor data
                myTimerEvent();
                }
                //Serial.println("- Loop4 - Blink stuff");
                vTaskDelay(3000);
        }
}
/////////////################# funciton  #####################////////////////
void IRAM_ATTR isr() {

        current_time=micros();
        if( digitalRead(MHZ19_PWM_PIN) == HIGH ) {
                LowLevel = current_time - StartLow;
                HighLevel = StartLow - StartHigh;
                StartHigh = current_time;
                NEW_DATA = true;
        }
        else{
                StartLow = current_time;
        }

}
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
void checkIaqSensorStatus(void)
{
        if (iaqSensor.status != BSEC_OK) {
                if (iaqSensor.status < BSEC_OK) {
                        output = "BSEC error code : " + String(iaqSensor.status);
                        Serial.println(output);
                        vTaskDelay(1000);
                        //for (;;);
                        //errLeds(); /* Halt in case of failure */
                } else {
                        output = "BSEC warning code : " + String(iaqSensor.status);
                        Serial.println(output);
                        vTaskDelay(1000);
                }
        }

        if (iaqSensor.bme680Status != BME680_OK) {
                if (iaqSensor.bme680Status < BME680_OK) {
                        output = "BME680 error code : " + String(iaqSensor.bme680Status);
                        Serial.println(output);
                        vTaskDelay(1000);
                } else {
                        output = "BME680 warning code : " + String(iaqSensor.bme680Status);
                        Serial.println(output);
                        vTaskDelay(1000);
                }
        }
        //iaqSensor.status = BSEC_OK;
}
/////////////################# funciton  #####################////////////////
void loadState(void)
{
        //Serial.println("EEMPROM[0]="+String(EEPROM.read(0)));
        if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
                // Existing state in EEPROM
                Serial.println(dateString);
                Serial.println("Reading state from EEPROM");

                for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
                        bsecState[i] = EEPROM.read(i + 1);
                        Serial.print(bsecState[i], HEX);
                        Serial.print(" ");
                }
                Serial.println(" ");

                iaqSensor.setState(bsecState);
                checkIaqSensorStatus();
        } else {
                // Erase the EEPROM with zeroes
                Serial.println("Erasing EEPROM");

                for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE + 1; i++)
                        EEPROM.write(i, 0);

                EEPROM.commit();
        }
}
/////////////################# funciton  #####################////////////////
void updateState(void)
{
        bool update = false;
        /* Set a trigger to save the state. Here, the state is saved every STATE_SAVE_PERIOD with the first state being saved once the algorithm achieves full calibration, i.e. iaqAccuracy = 3 */
        if (stateUpdateCounter == 0) {
                if (iaqSensor.iaqAccuracy >= 3) {
                        update = true;
                        stateUpdateCounter++;
                }
        }
        else {
                /* Update every STATE_SAVE_PERIOD milliseconds */
                if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
                        update = true;
                        stateUpdateCounter++;
                }
        }

        if (update) {
                iaqSensor.getState(bsecState);
                checkIaqSensorStatus();
                Serial.println(dateString);
                Serial.println("Writing state to EEPROM");

                for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
                        EEPROM.write(i + 1, bsecState[i]);
                        Serial.print(bsecState[i], HEX);
                        Serial.print(" ");
                }
                Serial.println("------");
                EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE );
                EEPROM.commit();
        }

}
/////////////################# Load variables to the app  #####################////////////////
void myTimerEvent()
{

        property = get_properties_for_iaq_level(varIaq);               // gets correspondig led color, led intensity and iaq description
        Blynk.virtualWrite(V34,varTemp);                        // sends temperature to chart of the app
        Blynk.virtualWrite(V25,varTemp);                        //sends temperature to numerical display of the app
        Blynk.virtualWrite(V33,varPres/100);                                    //sends pressure to the Blynk_App_Asistent
        Blynk.virtualWrite(V32,varHumi);                           //sends humidity
        Blynk.virtualWrite(V31,varCo2M);                            // sends co2 estimate or meassuerement
        Blynk.virtualWrite(V30,varGasP);             // sents gas percentage
        Blynk.virtualWrite(V28,varIaq);                                //sends iaq vale
        Blynk.virtualWrite(V29,iaq_to_percentage(varIaqAcc));     //sends iaq accuracy
        Blynk.setProperty(V27,"color",property.LED_color);                    //sends led color
        Blynk.virtualWrite(V27,property.LED_intensity);                       //sends led intensity
        Blynk.virtualWrite(V26,property.iaq_level_description);               //sends iaq description
        Blynk.virtualWrite(V24,dateString);                                    // sends time
        Blynk.virtualWrite(V23,fechaString);
        Blynk.virtualWrite(V21,wifiData_in_main.wifiName_for_main);
        Blynk.virtualWrite(V22,wifiData_in_main.wifiPassword_for_main);                      //sends date
        // and the engineer saw that the code..
        //                                                ...it was mehhh...
}//
////////////////////Touch_LCD/////////////////////////////////////////////////////////
void LCD_vars(){
        var_data.date_for_LCD=fechaString;
        //var_data.time_for_LCD=getZeit();
        var_data.time_for_LCD=dateString;
        var_data.iaq_for_LCD=varIaq;
        var_data.temp_for_LCD=varTemp;
        var_data.co2_for_LCD=varCo2M;
        var_data.gas_for_LCD=varGasP;
        var_data.pres_for_LCD=varPres/100;
        var_data.humi_for_LCD=varHumi;
}
//////////////////////////////////////////////////////////////////////////
//
/////////////################# funciton  #####################////////////////
void handleRoot()
{
        server.sendHeader("Location", "/index.html", true);
        server.send(302, "text/plane", "Hola Hola Hola");
}
/////////////################# funciton  #####################////////////////
bool loadFromSpiffs(String path)
{
        String dataType = "text/plain";

        if(path.endsWith("/"))
                path += "index.htm";

        if(path.endsWith(".src"))
                path = path.substring(0, path.lastIndexOf("."));

        else if(path.endsWith(".html"))
                dataType = "text/html";


        else if(path.endsWith(".css"))
                dataType = "text/css";

        else if(path.endsWith(".js"))
                dataType = "application/javascript";

        File dataFile = SPIFFS.open(path.c_str(), "r");
        if(server.hasArg("download"))
                dataType = "application/octet-stream";
        if(server.streamFile(dataFile, dataType) != dataFile.size())
        {

        }

        dataFile.close();
        return true;
}
/////////////################# funciton  #####################////////////////
void handleWebRequests()
{
        if(loadFromSpiffs(server.uri())) return;

        String message = "File not detected\n\n";
        message += "URI: ";
        message += server.uri();
        message += "\nMethod: ";
        message += (server.method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server.args();
        message += "\n";

        for(uint8_t i=0; i<server.args(); i++)
        {
                message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
        }

        server.send(404, "text/plain", message);
        Serial.println(message);
}
/////////////################# funciton  #####################////////////////
void dataSensorRequest(){

        if(WiFi.status() == WL_CONNECTED){
        dateString = getZeit();
        fechaString = getDatum(IN_NUMBERS);
        }
        varTemp = iaqSensor.temperature;
        varPres = iaqSensor.pressure;
        varHumi = iaqSensor.humidity;
        varGasR = iaqSensor.gasResistance;
        varCo2E = iaqSensor.co2Equivalent;
        varBVoc = iaqSensor.breathVocEquivalent;
        varGasP  = iaqSensor.gasPercentage;
        varIaq  = iaqSensor.iaq;
        varSiaq  =  iaqSensor.staticIaq;
        varStab  =  iaqSensor.stabStatus;
        varRunI =  iaqSensor.runInStatus;
        varIaqAcc =  iaqSensor.iaqAccuracy;
        varSiaqAcc  =  iaqSensor.staticIaqAccuracy;
        varCo2eAcc  =  iaqSensor.co2Accuracy;
        varBVocAcc  =  iaqSensor.breathVocAccuracy;
        varGasPAcc  =  iaqSensor.gasPercentageAcccuracy;


}
/////////////################# funciton  #####################////////////////
void JsonStringFormat(){

        cadena_envio = StartJS;//Initializer JSON string
        cadena_envio += String(varTemp) + SpacerJS;//temp
        cadena_envio += String(varPres/100) + SpacerJS;//pres
        cadena_envio += String(varHumi) + SpacerJS;//hum
        cadena_envio += fechaString+" "+ dateString+ SpacerJS;//fecha
        cadena_envio += messages_runin_stat[varRunI] +SpacerJS;//run in status
        cadena_envio += String(varCo2M) + SpacerJS; //co2 mess
        cadena_envio += String(varCo2E) + SpacerJS; //co2 stimation
        cadena_envio += messages_accuracy[varCo2eAcc] + SpacerJS; //co2 stimation accuracy
        cadena_envio += "Any suggestion" + SpacerJS; //co2 suggested acti
        cadena_envio += String(varBVoc) + SpacerJS;//breath VOC
        cadena_envio += messages_accuracy[varBVocAcc] + SpacerJS;//breath VOC accuracy
        cadena_envio += String(varGasP) + SpacerJS;//Gas percentage
        cadena_envio += messages_accuracy[varGasPAcc] + SpacerJS;//Gas percentage accuracy
        cadena_envio += String(varIaq)+ SpacerJS;//IAQ
        cadena_envio += messages_accuracy[varIaqAcc]+ SpacerJS;//IAQ Accuracy
        cadena_envio += messages_impact[iaq_Index2Level(varIaq)]+ SpacerJS;//IAQ Impact
        cadena_envio += messages_saction[iaq_Index2Level(varIaq)]+ SpacerJS;//IAQ Suggested actions
        cadena_envio += messages_iaqcolors[iaq_Index2Level(varIaq)]+ SpacerJS;
        cadena_envio += messages_quality[iaq_Index2Level(varIaq)]+ SpacerJS;
        cadena_envio += start_time;//+ SpacerJS;
        cadena_envio += StopJS;//Finalizer JSON string

}
/////////////################# funciton to print values on screnn  #####################////////////////

/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////

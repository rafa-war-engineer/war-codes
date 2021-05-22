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

#include <Arduino.h>
//#include <RTClib.h>
#include <Wire.h>
// #include <Adafruit_Sensor.h>
// #include "Adafruit_BME680.h"
#include <EEPROM.h>
#include "bsec.h"
#include <SPIFFS.h>
#include <WiFi.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
#include "WebServer.h"

#include "time.h"
#include <stdlib.h> //For future use in weather forecasting
#include <stdio.h> //For future use in weather forecasting
#include <BlynkSimpleEsp32.h>

//#include "esp_task_wdt.h"
//#include "soc/timer_group_struct.h"
//#include "soc/timer_group_reg.h"
#include "WeatherStat_NTP.h"
#include "WeatherStat_BlynkApp.h"
#include "WeatherStat_CO2sensor.h"
#include "WeatherStat_Messages.h"
//#include <DS3231.h>
/////////////################# directives #####################////////////////
#define Number_susc_sens 11
#define STATE_SAVE_PERIOD UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day
/////////////################# variables #####################////////////////
//char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
const byte led_gpio = 32; // the PWM pin the LED is attached to
int PWMchannel = 0;
int i = 0;
char flag=0;
//const char * intro = "Time[ms],r_t[°C],p[hPa],r_hum[%],gas[Ohm],IAQ,IAQacc,temp[°C],h[%],S_IAQ,CO2_equ,bre_VOC,Gas%";
const char * intro = "Time[ms],p[hPa],gas[Ohm],IAQ,temp[°C],h[%],CO2_equ,CO2,bre_VOC,Gas%,stabSta,runInSta";
uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};
uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t stateUpdateCounter = 0;
String output;
float varTemp, varPres, varHumi, varGasR, varCo2E, varBVoc, varGasP,varSiaq,varIaq;
int varIaqAcc, varStab, varRunI,   varSiaqAcc, varCo2eAcc;
int varBVocAcc, varGasPAcc;
float varCo2M=555;
String cadena_envio,dateString;

/////////////################# functions #####################////////////////
void loop2(void *parameter);
void loop1(void *parameter);
void feedTheDog(void);
void hw_wdt_disable(void);
void checkIaqSensorStatus(void);
void loadState(void);
void updateState(void);
void myTimerEvent(void);
void handleRoot();
bool loadFromSpiffs(String path);
void handleWebRequests();
void dataSensorRequest(void);
void JsonStringFormat(void);
/////////////################# handlers definition #####################///////
/////////////################# Web Specific #####################///////
// char *ssid = "FRITZ!Box 6591 Cable SW";         // replace with your SSID
// char *password = "62407078731195560963";
char *ssid = "FRITZ!Box 6591 Cable BE";          // replace with your SSID
char *password = "07225443701792235194";  // replace with your Password
//char *ssid = "Vodafone-FC6F = FRITZ!Box 6591 Cable BE";          // replace with your SSID/char *password = "07225443701792235194";  // replace with your Password";          // replace with your SSID
//char *password = "pW6298625330626571";  // replace with your Password
// AsyncWebServer server(80);
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

/////////////################# CO2_sensor ###################///////

/////////////################# Arduino sh%& #####################///////////////
TaskHandle_t Task2;
TaskHandle_t Task1;
//RTC_DS3231 rtc;
Bsec iaqSensor;
/////////////################# SETUP #####################////////////////
void setup() {
        Serial.begin(115200);
        Wire.begin();
/////RTC begin and check
        // if (!rtc.begin()) {
        //         Serial.println("Couldn't find RTC");
        //         while (1);
        // }
        // else{
        //         rtc.adjust(DateTime(__DATE__, __TIME__));
        //         Serial.println("RTC Found");
        // }

///SPIFSSS begin and check
        if(!SPIFFS.begin()) {
                Serial.println("An Error has occurred while mounting SPIFFS");
                return;
        }
        else Serial.println("SPIFFS Mounted correctly");

/////PWM Settings
        ledcAttachPin(led_gpio, PWMchannel); // assign a led pins to a channel
        ledcSetup(PWMchannel, 10000, 10); // 12 kHz PWM, 8-bit resolution

////// Wifi Configs
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
                delay(1000);
                Serial.print("Connecting to WiFi..");
                Serial.print(".");
        }
        Serial.print("\nConnected to the WiFi network: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP address:");
        Serial.print(WiFi.localIP());
////// Blynk begin config
        Blynk.begin(auth, ssid, password);
        timer.setInterval(1000L, myTimerEvent);
////// Config for readding CO2_sensor
        pinMode(MHZ19_PWM_PIN, INPUT);        //MHZ19 PWM Pin als Eingang konfigurieren
/////Initialize BME680 Sensor
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

/////Initialize BME680 Sensor
        iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
        output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
        Serial.println(output);
        //checkIaqSensorStatus();
        iaqSensor.setState(bsec_config_iaq);
        //checkIaqSensorStatus();
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
        //Serial.println("------### Entrando en loops");
/////Enabling Multicore program execution
        xTaskCreatePinnedToCore(loop1,"Task_1",10000,NULL,1,&Task1,0);
        delay(500);
        xTaskCreatePinnedToCore(loop2,"Task_2",10000,NULL,1,&Task2,1);
        delay(500);
        //hw_wdt_disable();

/////Handlers
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
        server.begin();// begin server at port 80
}
/////////////################# LOOP Executed in Core 0 #####################////////////////
void loop2(void *parameter) {

        while(1) {
                yield();
                Blynk.run(); // run code of the app
                timer.run();  // establishes comunication to the app in a time interval to load sensor data
                //Serial.print("---### LOOP2");
                //Serial.println(i);//
                //unsigned long time_trigger = millis();
                if (iaqSensor.run()) {   // If new data is available
                        dataSensorRequest();
                        JsonStringFormat();
                        Serial.println(cadena_envio);
                        // output = dateString;
                        // output += ", " + String(varPres/100);
                        // output += ", " + String(varGasR);
                        // output += ", " + String(varIaq);
                        // output += ", " + String(varTemp);
                        // output += ", " + String(varHumi);
                        // output += ", " + String(varCo2E);
                        // output += ", " + String(varCo2M);
                        // output += ", " + String(varBVoc);
                        // output += ", " + String(varGasP);
                        // output += ", " + String(varStab);
                        // output += ", " + String(varRunI);
                        //
                        // Serial.println( getDatum(IN_NUMBERS) );
                        // Serial.println(intro);
                        // Serial.println(output);
                        // Serial.println("Status: "+String( messages_runin_stat[varRunI] ));
                        // Serial.println("IAQ = "+String(varIaq));
                        // Serial.print(" Quality: ");
                        // Serial.println( messages_quality[iaq_Index2Level(varIaq)]);
                        // Serial.print(" Impact: ");
                        // Serial.println(messages_impact[iaq_Index2Level(varIaq)]);
                        // Serial.print(" Suggested action: ");
                        // Serial.println(messages_saction[iaq_Index2Level(varIaq)]);
                        // Serial.println("IAQ Accuracy = "+String(varIaqAcc));
                        // Serial.println(messages_accuracy[varIaqAcc]);
                        // Serial.print("IAQ_Static = "+String(varSiaq));
                        // Serial.println(" IAQ_Static Accuracy = "+String(varSiaqAcc));
                        // Serial.print("co2 = "+String(varCo2E)+" ppm");
                        // Serial.println(" co2 Accuracy = "+String(varCo2eAcc));
                        // Serial.print("breath-VOC = "+String(varBVoc)+" ppm");
                        // Serial.println(" breath-VOC Accuracy = "+String(varBVocAcc));
                        // Serial.print("Gas Percent = "+String(varGasP)+" %");
                        // Serial.println(" Gas Percent Accuracy = "+String(varGasPAcc));
                        // Serial.println(' ');
                        updateState();

                } else {
                        checkIaqSensorStatus();
                }

                server.handleClient();/// NO QUITAR!!!

        }///////// main while loop


}
/////////////################# LOOP Executed in Core 1 #####################////////////////
void loop1(void *parameter) {
        //Serial.print("---------### LOOP1");
        //Serial.println(i);//
        while(1) {
                yield();
                ledcWrite(PWMchannel, i);
                //Serial.println(BSEC_MAX_STATE_BLOB_SIZE);
                //delay(100);
                if (!flag) {
                        i++;
                        delay(1);
                }

                else{
                        i--;
                        delay(1);
                }

                if(i==1023) {
                        flag=1;
                        delay(75);
                }
                if(i==0) {
                        flag=0;
                        ledcWrite(PWMchannel, 0);
                        yield();
                        delay(600);
                        //Serial.println("Loop1 says: Core "+String(xPortGetCoreID()));
                }
        }
        //Serial.println("####Saliendo loop()");//
}
/////////////################# funciton  #####################////////////////
void loop(){
}
/////////////################# funciton  #####################////////////////
void feedTheDog(){
        // feed dog 0
        // TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
        // TIMERG0.wdt_feed=1;                       // feed dog
        // TIMERG0.wdt_wprotect=0;                   // write protect
        // // feed dog 1
        // TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
        // TIMERG1.wdt_feed=1;                       // feed dog
        // TIMERG1.wdt_wprotect=0;                   // write protect
}
/////////////################# funciton  #####################////////////////
void hw_wdt_disable(){
        *((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF
}
/////////////################# funciton  #####################////////////////
void checkIaqSensorStatus(void)
{
        if (iaqSensor.status != BSEC_OK) {
                if (iaqSensor.status < BSEC_OK) {
                        output = "BSEC error code : " + String(iaqSensor.status);
                        Serial.println(output);
                        //for (;;);
                        //errLeds(); /* Halt in case of failure */
                } else {
                        output = "BSEC warning code : " + String(iaqSensor.status);
                        Serial.println(output);
                }
        }

        if (iaqSensor.bme680Status != BME680_OK) {
                if (iaqSensor.bme680Status < BME680_OK) {
                        output = "BME680 error code : " + String(iaqSensor.bme680Status);
                        Serial.println(output);
                } else {
                        output = "BME680 warning code : " + String(iaqSensor.bme680Status);
                        Serial.println(output);
                }
        }
        //iaqSensor.status = BSEC_OK;
}
/////////////################# funciton  #####################////////////////
void loadState(void)
{
        if (EEPROM.read(0) == BSEC_MAX_STATE_BLOB_SIZE) {
                // Existing state in EEPROM
                Serial.println("Reading state from EEPROM");

                for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
                        bsecState[i] = EEPROM.read(i + 1);
                        Serial.println(bsecState[i], HEX);
                }

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

                Serial.println("Writing state to EEPROM");

                for (uint8_t i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
                        EEPROM.write(i + 1, bsecState[i]);
                        Serial.println(bsecState[i], HEX);
                }

                EEPROM.write(0, BSEC_MAX_STATE_BLOB_SIZE);
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
        Blynk.virtualWrite(V24,getZeit());                                    // sends time
        Blynk.virtualWrite(V23,getDatum(IN_NUMBERS));                         //sends date

        // and the engineer saw that the code..
        //                                                ...it was mehhh...
}//
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
        message += (server.method() == HTTP_GET)?"GET":"POST";
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

        dateString = getZeit();
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

        cadena_envio = StartJS;//Initializer JSON chain
        cadena_envio += String(varTemp) + SpacerJS;//temp
        cadena_envio += String(varPres/100) + SpacerJS;//pres
        cadena_envio += String(varHumi) + SpacerJS;//hum
        cadena_envio += getDatum(IN_LETTERS)+" "+ dateString+ SpacerJS;//fecha
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
        cadena_envio += messages_quality[iaq_Index2Level(varIaq)];//+ SpacerJS;
        cadena_envio += StopJS;//Finalizer JSON chain

}
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////
/////////////################# funciton  #####################////////////////

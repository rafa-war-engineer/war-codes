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
 #include "WeatherStat_BlynkApp.h"          // name from the library created also knon als Brötchen des Tages
 #include "time.h"                          // time library for some reason


///////////////////////////////        funtion to conver the measure of acurracy from 0 to 4 to a percentage from 0 to 100/////////////////////////////////////////////////
int iaq_to_percentage(int iaq_1_to_4){
        int iaq_percentage = iaq_1_to_4*25;
        return(iaq_percentage);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////function to get led color led intencity and iaq description///////////////////////////////////////////////
iaq_level get_properties_for_iaq_level(int iaq_meassure){
        iaq_level property;

        if(iaq_meassure <= 50) {
                property.LED_color = green;
                property.LED_intensity = level_bright;
                property.iaq_level_description = "Excellent";
        }else if(iaq_meassure <= 100) {
                property.LED_color = green; //Green
                property.LED_intensity = level_dim;
                property.iaq_level_description = "Good";
        }else if(iaq_meassure <= 150) {
                property.LED_color = yellow; //Yellow
                property.LED_intensity = level_bright;
                property.iaq_level_description = "Lightly polluted";
        }else if(iaq_meassure <= 200) {
                property.LED_color = red; //Red
                property.LED_intensity = level_dim;
                property.iaq_level_description = "Moderately polluted";
        }else if(iaq_meassure <= 250) {
                property.LED_color = red; //Red
                property.LED_intensity = level_bright;
                property.iaq_level_description = "Heavily polluted";
        }else if(iaq_meassure <= 350) {
                property.LED_color = dark_blue; //Dark blue
                property.LED_intensity = level_dim;
                property.iaq_level_description = "Severely polluted";
        }else if(iaq_meassure >= 351) {
                property.LED_color = dark_blue; //Dark blue
                property.LED_intensity = level_bright;
                property.iaq_level_description = "Extremely polluted";
        }

        return(property);
}
////////////////////////////////////////////////////////////////////////////////

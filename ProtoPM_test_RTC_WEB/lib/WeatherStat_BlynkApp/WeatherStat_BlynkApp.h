/**
 * @Author: Rafael Aranda M.Sc Student FH Aachen- CIDESI <rafa>
 * @Date:   2021-27-04
 * @Email:  rafael.aranda@alumni.fh-aachen.de
 * @Filename: WeatherStat_BlynkApp.h
 * @Last modified by:   rafa
 * @Last modified time: 2021-27-04
 * @License: CC by-sa
 */
//////////// Tutorial WeatherStat_BlynkApp_h /////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this library contains two functions
//
// the function:
//                p = iaq_to_percentage(x);
//
// will reurn the value of the input times 25
//
//               p = x*25
//
// both "p" and "x" are integers
//
// the function:
//                q = get_properties_for_iaq_level(iaq_value);
//
// will reurn a structure whith the corresponding values of led_color led_intensity and iaq_description to the iaq_value of the input
//
//                q ={string = led_color,int = led_intensity,string = iaq_description}
//
// to use this funtion in main q most also be declare in main(); to do this one mos write the declaration:
//
//               iaq_level q;
//
/////////////////////// end of tutortial WeatherStat_BlynkApp_h ////////////////////////////////////////////////////////////////////////////////////
#ifndef WeatherStat_BlynkApp_h //This goes allways Luke
#define WeatherStat_BlynkApp_h // this also
#include <Arduino.h>      // that one is optional in the .h i think
// difinitions for led colors and led intencitys of the library
#define green "#23C48E"
#define yellow "#ED9D00"
#define red "#D3435C"
#define dark_blue "#5F7CD8"
#define level_bright 255
#define level_dim 130
//////////// end of definitions

////////////////////////////////////////declaration of the structure for the return //////////////////////////////////////
struct iaq_level{
  String LED_color;
  int LED_intensity;
  String iaq_level_description;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////  declaration of functions /////////////////////////////////////////////////////////////
int iaq_to_percentage(int iaq_1_to_4);
iaq_level get_properties_for_iaq_level(int iaq_meassure);
#endif   // that goes allways it closes the library

/////////////////////////////////  end of the library ////////////////////////////////////////////////////////////////

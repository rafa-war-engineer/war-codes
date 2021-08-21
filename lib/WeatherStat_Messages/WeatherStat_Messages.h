/**
 * @Author: Daniel Murrieta M.Sc. Student FH Aachen - CIDESI <daniel>
 * @Date:   2020-05-02T23:38:27+02:00
 * @Email:  daniel.murrieta-alvarez@alumni.fh-aachen.de
 * @Filename: WeatherStat_Messages.h
 * @Last modified by:   daniel
 * @Last modified time: 2021-05-17T02:47:33+02:00
 * @License: CC by-sa
 */
#include<Arduino.h>
 #ifndef WeatherStat_Messages_h
 #define WeatherStat_Messages_h
 String StartJS = "[\"";
 String SpacerJS = "\",\"";
 String StopJS = "\"]";

const char *messages_quality[] = {
   "Excellent",
   "Good",
   "Lightly polluted",
   "Moderately polluted",
   "Heavily polluted",
   "Severely polluted",
   "Extremely polluted"
 };

const char *messages_impact [] = {
   "Best for well-being",
   "No impact on well-being",
   "Reduction of well-being possible",
   "More significant irritation possible",
   "Exposition might lead to effects like headache",
   "More severe health issue possible if harmful VOC present",
   "Headaches, additional neurotoxic effects possible"
 };
 //
const char *messages_saction [] = {
   "No measures needed",
   "No measures needed",
   "Ventilation suggested",
   "Increase ventilation with clean air",
   "Optimize ventilation",
   "Contamination should be identified, maximize ventilation and reduce attendance",
   "Contamination needs to be identified, avoid presence and maximize ventilation"
 };
 //
const char *messages_iaqcolors [] = {
   "#80E335",
   "#A5CF5B",
   "#FEFB3B",
   "#E37D1D",
   "#DC2A04",
   "#831D4C",
   "#5B3606"
 };
 //  char *messages_iaqcolors [] = {
 //   "80E335",
 //   "A5CF5B",
 //   "FEFB3B",
 //   "E37D1D",
 //   "DC2A04",
 //   "831D4C",
 //   "5B3606"
 // };
 //
const char *messages_accuracy [] = {
   "Stabilization, run-in ongoing",
   "Low accuracy, auto-trimming on-going",
   "Medium accuracy, auto-trimming ongoing",
   "High accuracy"
 };

const char *messages_runin_stat[] = {
   " Power-on stabilization ongoing",
   " Power-on stabilization finished"
 };

const char *messages_stab_stat[] = {
   "Initial sensor stabilization ongoing",
   "Initial sensor stabilization finished"
 };

const char *messages_co2_description[] = {
   "Normal background concentration in outdoor ambient air",
   "Concentrations typical of occupied indoor spaces with good air exchange",
   "Complaints of drowsiness and poor air",
   "Headaches, sleepiness and stagnant, stale, stuffy air. Poor concentration, loss of attention, increased heart rate and slight nausea may also be present"
 };

const char *messages_conn[] = {
   "•Connected to network: ",
   "•Wireless Access point: ",
   " WheatherStat_AP",
   "•IP: ",
   "10.10.10.10",
   "•Blynk App available",
   "•Blynk App not available"
 };

  uint8_t co2_Level2Descrip(int level);
  uint8_t iaq_Index2Level(int index);
  char * strcpy ( char * destination, String source );
  //char * strcpy(char * destination, char * source);
 #endif   // that goes allways it closes the library

# WeatherStat v1.1
Personal Weather Station - Mechatronik Projekt 2020 Bosch BME680

Working Stuff:
- 2 core execution
- Bosch BSEC Library for obtainign messures and estimations
- Asynchronous Web Server
- Saving objects html, js and css in flash with SPIFFS
- Saving state of the sensor in EEMPROM, to restoring after reset
- NTP Client to get date and time.

Not Working:


TODO:
- Further development of the webserver, graphically and aestetically speaking.
- HASL compensated calculation.
- Addition of other parts  of the proyect

The source code is in src folder, and is possible to change termination from cpp to ino.
The Webserver files are in data folder, thre are three of them.
The state config files are in config folder.
The custom libraries are in lib folder.

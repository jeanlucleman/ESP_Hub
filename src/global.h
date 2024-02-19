#ifndef GLOBAL_H
#define GLOBAL_H

#define ESP_8266
#include "settings.h"
#include "doorSensor.h"
#define DEBUG                           // Comment this line to disable debug
#ifdef DEBUG
  #define debug(x) Serial.print(x);
  #define debugln(x) Serial.println(x);
  #define debugf2(x,y) Serial.printf(x,y);
  #define debugf3(x,y,z) Serial.printf(x,y,z);
  #define debugf4(x,y,z,w) Serial.printf(x,y,z,w);
  #define debugf6(a,b,c,d,e,f) Serial.printf(a,b,c,d,e,f)
  #define debugInit(baud) Serial.begin(baud)
  #define debugDated(x) printDated(x)
  #define debuglnDated(x) printlnDated(x)
  
#else
  #define debug(x) // do nothing
  #define debugln(x)
  #define debugf2(x,y)
  #define debugf3(x,y,z)
  #define debugf4(x,y,z,w) 
  #define debugf6(a,b,c,d,e,f)
  #define debugInit(x)
  #define debugDated(x) 
  #define debuglnDated(x) 
#endif

//#define OTA
bool initialisations();
extern Settings commonData;
extern Settings colSensors;

#endif
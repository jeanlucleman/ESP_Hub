//MyFunctions.cpp
// -------------------INCLUDES ------------------------
  
  #include <Arduino.h>
  #include "global.h"
  
  #include <ESPAsyncWebServer.h>
  // #include <time.h>
  // #include "functions.h"

  #ifdef ESP_8266
  #include "FS.h"
  #elif defined ESP_32
  #include <SPIFFS.h>
  #endif


  //#include "data.h"
 // #include "flashData.h"

// Définition des GPIO

  // Led wifiLed(0,OFF,0);
  // #ifdef REDLED
  //   Led redLed(0,OFF,0);
  // #endif
  // vector<Led *> listLeds; // Le tableau listLeds contient les pointeurs vers les objets de type Led

  
  int pinButton;

  // Settings deviceData("settings");
  

  Settings commonData("commonSettings");
  Settings colSensors("colSensors");
  // String commonDataFileName="commonSettings";
  // bool setupLeds() 
  //   {  
  //   //  ------------Initialisation des pins-----------
  //     #ifdef REDLED
  //       int redLedPinNumber=deviceData.getValue("redLedPinNumber").toInt();
  //       redLed.setPin(redLedPinNumber);
  //       redLed.name="Red LED";
  //       redLed.setPeriod(50);
  //       redLed.blinkMaxCount=20;
  //       redLed.setMode(NOACTION);
  //       Serial.printf("LED %d ON\n", redLedPinNumber);
  //     #endif
  //     int pinWifiLed=deviceData.getValue("wifiLedPinNumber").toInt();
  //     if (pinWifiLed<0){
  //       wifiLed.inverted=true;
  //       pinWifiLed=-pinWifiLed;
  //     }
  //     #ifdef RELAY
  //     pinRelay=deviceData.getValue("relayPinNumber").toInt();
  //     pinMode(pinRelay,OUTPUT);
  //     #endif
  //     #ifdef PUSHBUTTON
  //       pinButton=deviceData.getValue("pushButtonPinNumber").toInt();
  //       pinMode(pinButton, INPUT);
  //     #endif
  //     wifiLed.setPin(pinWifiLed);
  //     #ifndef TFT
  //       //pinMode(pinWifiLed, OUTPUT);
  //     #endif
  //     wifiLed.setPeriod(500);
  //     wifiLed.setMode(FLASH);
  //     Serial.print("Nombre de LED: ");
  //     Serial.println(Led::getCount());
  //     wifiLed.name="Blue LED";
  //     //redLed.name="Led rouge";
  //     listLeds.push_back(&wifiLed); // On ajoute au tableau des pointeurs de Led l'adresse de chaque objet de type Led.
  //     #ifdef REDLED
  //       listLeds.push_back(&redLed);
  //     #endif
  //     for (unsigned int i=0; i<listLeds.size();i++)
  //       {
  //         Serial.print("Numéro de pin pour la " + listLeds[i]->name + ": ");
  //         Serial.println(listLeds[i]->getPin()); // on utilise un pointeur d'où ->
  //       }
  //     delay(1000);
  //     return true;
  // }

bool initialisations()
  {
    try
      {
        // Serial.begin(74880);
        debugln("\n");
        debugln("Début des initialisations...");
        delay(500);
      }
    catch(...)
      {
        return false;
      } 
    #ifdef ESP_8266
    debugln("ESP board = ESP8266");
    #elif defined ESP_32
    debugln("ESP board = ESP32");
    #endif
    commonData.ini();  // L'initialisation de cet objet n'est pas fait lors de l'instantiation. La raison est que celle-ci se déroule 
    // dès la déclaration du type de l'objet (Settins myData;) et qu'à ce stade toutes les initialisations de l'ESP ne sont pas terminées
    // ce qui crée un plantage.
    debugln("Listing des settings:");
    commonData.list();
    colSensors.ini();
    colSensors.list();
    DoorSensor::doorSensorCount=0;
    for (size_t i = 0; i < colSensors.count(); i++)
    {
        doorSensor[i]=DoorSensor();
        doorSensor[i].setDoorSensor(colSensors.getKey(i));
    }
    
    

    return true;
  }


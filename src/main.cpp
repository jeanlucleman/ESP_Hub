/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* ESP8266                                   Version 1.0 01/2024*
*                                                                                      *
* This code setup an ESP8266 as a UDP Access Point (Server) that receives    *
* the On/Off state of a switch connected to ESP8266 ESP-01 module that is set  *
* up as a UDP Client.                                *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
TODO
Le doorsensor doit aussi envoyé sa version de firmware pour l'afficher dans la page liste des détecteurs

*/
//Libraries Included
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266HTTPClient.h>
#include "doorSensor.h"   // Class for door sensor object
#include "global.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <NTPClient.h>  // voir https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
#include "clock.h"
#include "functions.h"
#include "serialCom.h" // To wire communicate with another ESP to get information on presenec (by phone detection of BLE device)
#ifdef OTA
  #include <ArduinoOTA.h>
#endif
#include <version.h> 
//Authentication Variables that will be used by the UDP clients (doorSensores)
String AP_SSID, AP_PSW, STA_SSID, STA_PSW; 
// String     AP_SSID;                  // WIFI Name of this Access Point giving access to the ESP8266 though AP mode (ex. 192.168.4.1)
// String     AP_PSW;                   // Password to access this AP
//Authentication Variables that will be used by the http clients (users)
// String STA_SSID;                      // WIFI Name of the router allowing access to the ESP8266 through the home router (ex. 192.168.2.98)
// String STA_PSW;                       // PSW of home router
//WiFi settings for the AP mode
IPAddress APlocal_IP(192, 168, 4, 10);   //Access Point (AP) Local IP Address
IPAddress APgateway(192, 168, 4, 1);     //Access Point (AP) Gateway Address
IPAddress APsubnet(255, 255, 255, 0);    //Access Point (AP) Subnet Address
unsigned int UDPPort = 9999;             //Local port to listen on
WiFiUDP Udp;
WiFiUDP UdpClock; // It must be another instance of WiFiUDP otherwise after clock setup, the UDP transmission with swithes is lost. 
// Wifi settings for the STA mode: read from the setting file. Settings can be acccessed by a connection on AP mode (ex. 192.168.4.1)
bool staModeOK=false;  // Report success of STA connexion
/*--------Time and date management---- */
unsigned long currentMillis;
unsigned long lastTimeRequestMillis;
NTPClient timeClient(UdpClock, "pool.ntp.org",3600,3600); // works only if STA mode OK. Time offset 1 hour, update each hour. See https://randomnerdtutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
Clock* myClock=Clock::getInstance(); // Creates a clock. It gives the current date/time by date/time at t0 + millis elapsed since t0.
  // t0 is the date/time given by a call to OTP at t0
/* -----------------------------------*/
AsyncWebServer * server;                // Server creation (it can be access from AP of STA mode)
String pageName;                        // Nom de la page html à afficher (appelé dans templateSTA.html, ex. info => info.html qui est dans le répertoire data, )
 
//Other Variables
char packetBuffer[255];                 //buffer to hold incoming packet from UDP exchanges
char notReceived[4] = "BAD";
char received[4] = "ACK";
char* AckMsg;                           // Acknowledge message sent back to Client
bool rebootRequested=false;             // When the server receive /reboot, this boolean is set to true and is read in the loop
bool presence=true;                     // Indicates if an authorized person is present inside
bool alarmActionRequested=false;        // Indicates to the loop if an action has to be set (ex. hooter)
/* #region Fonctions */
  void handleClients();                   // Check if there is UDP clients and read, if any, incoming data
  void setupWifi();                       // Start wifi in AP and STA modes
  void startServer();                     // Start the server on this ESP8266
  void checkWifi();                       // Check wifi status and setup wifi when not connected 
  void printDated(String text);           // Format serial output with date
  void printlnDated(String text);         // Format serial output with date and CR
  bool setupClock();                      // Initialize a NTP client to connect on a NTP server to get time and date
  bool setClockFromOTP();                 // Get time and date from a NTP client and update the clock (see Clock class)
  void maintainClock();                   // Periodically check the clock and update
  bool startStaMode();
/* #endregion */
//====================================================================================
void setup() 
  {
    Serial.begin(74880);                       //Set Serial Port to 115200 baud
    if (!initialisations())
      {
        debugln(F("Initialization failed!"));
        return;
      }
    WiFi.disconnect();                                  //Stop active previous WIFI
    checkWifi();           // Function to check wifi status and to setup wifi when not connected
    setupClock(); 
    #ifdef OTA
      ArduinoOTA.setHostname("ESP8266");
      ArduinoOTA.onStart([]() {
        debugln("Start");
      });
      ArduinoOTA.onEnd([]() {
        debugln("\nEnd");
      });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        debugf2("Progress: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([](ota_error_t error) {
        debugf2("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) debugln("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) debugln("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) debugln("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) debugln("Receive Failed");
        else if (error == OTA_END_ERROR) debugln("End Failed");
      });
      ArduinoOTA.begin();
    #endif
    debuglnDated("Setup completed!");
  }
//====================================================================================
void loop() 
  {
    currentMillis=millis();
    #ifdef OTA
      ArduinoOTA.handle();
    #endif
    handleClients();            // Function to handle UDP client, receiving info from door sensors
    maintainClock();            // Mise à jour de l'horloge une fois par heure (ou toutes les min si l'heure est manifestement fausse) */
    if(rebootRequested){ESP.restart();}
  }

//=================================  SUBROUTINES  ====================================

void handleClients()
  {
    int packetSize = Udp.parsePacket();                    //Check for received data in the  packet
    if (packetSize) 
      {                                      //Is there data in the packet?
        debug("Received packet of size ");
        debug(packetSize);
        debug(" from ");
        debug(Udp.remoteIP());
        debug(", port ");
        debugln(Udp.remotePort());
        int len = Udp.read(packetBuffer, 255);  //Read the packet into packetBufffer with an expected
                                                // format  Switch=xyy:yy:yy:yy:yy:yy where x = 0 or 1 and yy:yy... is the MAC address of esp01 switch

        if (len > 0) 
          {
            packetBuffer[len] = 0;                             //Add a null character to end of packet
          }
        debugln(packetBuffer);
        //Check received data and send appropriate acknowledge back
        AckMsg = notReceived;                                  //Preset acknowledge message to 'BAD'
        char * myArray[5];//https://stackoverflow.com/questions/27325964/splitting-char-array
        int index=0;
        char *token;
        const char s[2] = ",";
        /* get the first token */
        token = strtok(packetBuffer, s);

        /* walk through other tokens */
        while( token != NULL ) 
        {
            myArray[index] = (char *)  malloc (sizeof(char ) *(strlen(token) + 1));
            strcpy(myArray[index],token);
            index ++;
            token = strtok(NULL, s);
        }
        debugln(myArray[0]);
        debugln(myArray[1]);
        debugln(myArray[2]);
        debugln(myArray[3]);

        // char* p;
        // p=strstr(packetBuffer,"Switch=");
        if(strcmp(myArray[0],"Switch")==0)
          {
            debugln("This packet is sent from a door sensor");
            String myMac= myArray[2]; //String(packetBuffer).substring(8,25);
            getDoorSensor(myMac)->firmware=myArray[3];
            if (getDoorSensor(myMac)->setState(myArray[1][0]))       // Report true if there is state changed
              {
                size_t state= getDoorSensor(myMac)->getState();
                debug("The new state is ");
                debugln((state!=enmSwitchState::closed)?"Closed":"Open");
                debugln("--------------------------------------------------------------");
                // Place here action needed when switch state changes
                alarmActionRequested=(state==enmSwitchState::open?true:false);
              }
            else{debugln("No change");}
            AckMsg = received; 
          }
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());   //Send acknowledge response packet to client
        Udp.write(AckMsg);
        Udp.endPacket();
    }
  }
void getCredentials() //String& AP_SSID, String& AP_PSW, String& STA_SSID, String& STA_PSW) // Read wifi credential from settings file
  {
    AP_SSID  = commonData.getValue("AP_SSID");
    AP_PSW   = commonData.getValue("AP_PSW");
    STA_SSID = commonData.getValue("STA_SSID");
    STA_PSW  = commonData.getValue("STA_PSW");
  }
void checkWifi()      // Function to check wifi status and to setup wifi when not connected
  {
    if (WiFi.isConnected() and WiFi.status()==WL_CONNECTED)
      {
        debugln("Wifi is connected!");
      }
    else
      {
        debugln("Wifi is not connected");
        setupWifi();        
      }
  }
void setupWifi(){                

  getCredentials();//AP_SSID,AP_PSW, STA_SSID, STA_PSW);  // Gets credentials for AP and STA configuration
  WiFi.mode(WIFI_AP_STA);                             //Set WiFi mode to AP and STA 
  debugln("WIFI Mode set to: AP Station");
  // Configuring AP
  WiFi.softAPConfig(APlocal_IP, APgateway, APsubnet);  
  WiFi.softAP(AP_SSID.c_str(), AP_PSW.c_str(), 1 ,0, NBMAXDOORSENSOR);              //WiFi.softAP(ssid, password, channel, hidden, max_connection)                         
  debugln("WIFI Named " + AP_SSID + " started (Access point mode)"); //Send info to monitor if debug = true
  delay(50);                                             //Wait a bit
  IPAddress IP = WiFi.softAPIP();                        //Get server IP
  debug("AccessPoint IP: ");
  debugln(IP);
  // Starting UDP to exchange between switches and this ESP
  Udp.begin(UDPPort);                                    //Start UDP Server


  // Configuring STA
  staModeOK=startStaMode();
  startServer(); 
}

bool startStaMode()
  {
    IPAddress local_IP ; 
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns; // Cette ligne est nécessaire si on appelle des sites extérieurs (comme pool.ntp.ord).
    bool staModeSuccess;
    local_IP.fromString(commonData.getValue("staIp"));
    gateway.fromString(commonData.getValue("staGateway")); //192, 168, 2, 1);
    subnet.fromString(commonData.getValue("staSubnet")); //(255, 255, 0, 0);
    dns.fromString(commonData.getValue("staDNS")); //(192, 168, 2, 1); // Cette ligne est nécessaire si on appelle des sites extérieurs (comme pool.ntp.ord).
    if (!WiFi.config(local_IP, gateway, subnet, dns)) // Configures static IP address
      {
        debugln("STA Failed to configure");
      }
    WiFi.begin(STA_SSID.c_str(), STA_PSW.c_str());
    delay(100);
    debugln(F("Tentative de connexion en mode STA..."));
    unsigned long startMillis = millis();
    staModeSuccess=true;
    while (WiFi.status() != WL_CONNECTED)
      {
        yield();
        if (millis() > startMillis + 15000)
          {
            debugln(F("\nWifi connexion in STA mode timeout reached!"));
            staModeSuccess=false;
            break;
          }
        delay(1000);
        debug(".");
      }
    if(staModeSuccess)
      {
        debugln("\nAdresse IP sur le réseau : " + WiFi.localIP().toString());
        debugln("Adresse DNS              : " + WiFi.dnsIP().toString());
        debugln("Gateway                  : " + WiFi.gatewayIP().toString());
        debug("Connexion OK en mode STA sur ");
        debugln(WiFi.localIP());
      }
    // Starting server (if STA mode fails, is accessible in AP mode on 192.168.4.1)
    debug("Mac address: ")
    debugln(WiFi.macAddress());
    return staModeSuccess;
  }
/* #region Fonctions du serveur */

  String getFileContent(String fileName)
    {
      File file;
      #ifdef ESP_8266
        file = LittleFS.open("/" + fileName, "r");
      #elif defined ESP_32
        file = SPIFFS.open(fileName);
      #endif
      if(!file)
        {
          return "There was an error opening the file " +fileName;
        }
      String fileContent;
      //String lineRead;
      //while (file.available()) 
      //  {
      printlnDated("Reading file content from " + fileName);
      fileContent=file.readString();
      debuglnDated("File content: \n" + fileContent);
          //lineRead = file.readStringUntil('\n'); // one line should be in the form key,value,description
          //debugln("Ligne lue: " + lineRead);
          //lineRead.trim(); // to remove cr or other control character beside the string
          //fileContent = fileContent + lineRead;
      //  }
      file.close();
      debuglnDated("Closing file '" + fileName + "'");
      return fileContent;

    }
  // Cette fonction est appelée par une requête de type GET, sans argument, au chargement de la page files.html
  String getFileList()
    {
      debuglnDated("Entrée dans getFileList");
      String listFiles;
      #ifdef ESP_8266
      if (!LittleFS.begin())
      #elif defined ESP_32
      if (!SPIFFS.begin(false))
      #endif
      {
        debugln("An Error has occurred while mounting file system");
        return "";
      }
      String sep="$$";
      #ifdef ESP_8266
      Dir root = LittleFS.openDir("/");

      while (root.next()) 
        {
          listFiles += String(root.fileName()) + sep;
          if(root.fileSize()) 
            {
              File f = root.openFile("r");
              debugDated("Taille: ");
              debugln(f.size());
            }
        }


      #elif defined ESP_32
      File root = SPIFFS.open("/");
      File file = root.openNextFile();
      while (file)
      {
        listFiles += String(file.name()) + sep;
        file = root.openNextFile();
      }

      #endif
      debuglnDated("Return from getFileList");
      return listFiles;
    }
  
/***
 * Return the list of sensors from the existing and valid doorSensor objects
*/
    String getListSensors() //"000206,07:25,1,21:21,0";
      {
        debugln("getListSensors");
        String result="";
        for (size_t i = 0; i < DoorSensor::doorSensorCount; i++)
        {
          if(doorSensor[i].valid)
            {
              if(result!=""){result+="$$";}/* code */
              Serial.println(doorSensor[i].mac);
              result+=doorSensor[i].mac+"," 
              +doorSensor[i].name + "," 
              +doorSensor[i].description + ","
              +doorSensor[i].firmware;  
            }

        }
        debugln(result);
        return result;

      }
    String getSensorsStatus() //"000206,07:25,1,21:21,0";
      {
        // debugln("getSensorsStatus");
        String result="";
        for (size_t i = 0; i < DoorSensor::doorSensorCount; i++)
        {
          if(doorSensor[i].valid)
            {
              if(result!=""){result+="$$";}/* code */
              Serial.println(doorSensor[i].mac);
              result+=doorSensor[i].mac+"," 
              +doorSensor[i].getState();  
            }

        }
        // debugln(result);
        return result;

      }
  String processorConfig(const String &var)
    { // le & signifie que var est passé par référence
      if (var == "STAIP")
        {
          return commonData.getValue("staIp");
        }
      if (var == "MAINSSID")
        {
          return commonData.getValue("STA_SSID");
        }
      if (var == "MAINPSW")
        {
          return commonData.getValue("STA_PSW");
        }
      // if (var == "FALLBACKSSID")
      //   {
      //     return commonData.getValue("fallbackSSID");
      //   }
      // if (var == "FALLBACKPSW")
      //   {
      //     return commonData.getValue("fallbackPSW");
      //   }
      if (var == "STAGATEWAY")
        {
          return commonData.getValue("staGateway");
        }  
      if (var == "STASUBNET")
        {
          return commonData.getValue("staSubnet");
        }
      if (var == "STADNS")
        {
          return commonData.getValue("staDNS");
        }
      // if (var == "GMTOFFSET")
      //   {
      //     return commonData.getValue("gmtOffset");
      //   }
      // if (var == "DST")
      //   {
      //     return commonData.getValue("dst");
      //   }
      return "";
    }
  String processorTemplate(const String &var)
    { // le & signifie que var est passé par référence
      // if (var == "DEVICENAME")
      //   {
      //     //return commonData.getValue("espName");
      //     return "xxxxxx";
      //   }
      // if (var== "ESPNAME")
      //   {
      //     return "espname";
      //   }
      if (var == "DESCRIPTION")
        {
          //return commonData.getDescription("espName");
          return "Alarme";
        }
      if (var== "PAGENAME")
        {
          return pageName;
        }
      return "";
    }
  String processorInfo(const String &var)
    { // le & signifie que var est passé par référence
      // if (var == "ESPNAME")
      //   {
      //     return "xxxx";//deviceData.getValue("espName") + " (type: " + DEVICETYPE + ")" ;
      //     //return espName;
      //   }
      if (var=="FIRMWAREVERSION")
        {
          return VERSION;
        }
      if (var == "CURRENTIP")
        {
          return WiFi.localIP().toString();
        }
      if (var == "CURRENTSSID")
        {
          return WiFi.SSID();
        }
      // if (var == "CURRENTSTRENGTH")
      //   {
      //     return "TBD";
      //   }
      if (var == "CURRENTGATEWAY")
        {
          return WiFi.gatewayIP().toString();
        }
      if (var == "CURRENTSUBNET")
        {
          return WiFi.subnetMask().toString();
        }
      if (var == "CURRENTDNS")
        {
          return WiFi.dnsIP().toString();
        }
      if (var == "CURRENTAPIP")
        {
          return WiFi.softAPIP().toString();
        }
      if (var == "CURRENTAPMAC")
        {
          return WiFi.softAPmacAddress();
        }
      if (var == "CURRENTSTAMAC")
        {
          return WiFi.macAddress();
        }
      if (var=="LOCALTIME")
        {
          return myClock->getNow_YYYY_MM_DD_HH_MM_SS();
        }
      if (var=="UPTIME")
        {
          unsigned long uptime=millis()/1000;
          int days = uptime/86400;
          int hours = (uptime-86400*days)/3600;
          int minutes = (uptime -86400*days-3600*hours)/60;
          int secondes = uptime -86400*days-3600*hours - 60*minutes;
          return String(days)+"j " + String(hours)+ "h " + String(minutes) + "min "+ String(secondes) + "s";
        }
      if (var == "STARTTIME")
        {
          //unsigned long startMillis =13000;
          return myClock->getDateTime_YYYY_MM_DD_HH_MM_SS(0); // The clock counter start at millis = 0
        }
      return "";
    }
  String processorFiles(const String &var)
    { // le & signifie que var est passé par référence
      return "";
    }
  String processorSensors(const String &var)
    { 
      return "";
    }
  void startServer()
  {
    #ifdef ESP_32
      #define FILESYSTEM SPIFFS
    #elif defined ESP_8266
      #define FILESYSTEM LittleFS
    #endif
    server = new AsyncWebServer(80);
    server->on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/w3.css", "text/css");
      }
    );
    server->on("/functions.js", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/functions.js", "text/javascript");
      });
    server->on("/copyright.js", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/copyright.js", "text/javascript");
      });
    server->on("/settings.js", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/settings.js", "text/javascript");
      });
    server->on("/jquery-3.4.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/jquery-3.4.1.min.js", "text/javascript");
      });
    server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/favicon.png", "image/png");
      });
    server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        pageName="config";
        request->send(FILESYSTEM, "/templateSTA.html", String(), false, processorTemplate);
      });
    server->on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/config.html", String(), false, processorConfig);
      });
    server->on("/info.html", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
      request->send(FILESYSTEM, "/info.html", String(), false, processorInfo);
      });  
    server->on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        pageName="sensors";
        request->send(FILESYSTEM, "/templateSTA.html", String(), false, processorTemplate);
      });
    server->on("/sensors.html", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/sensors.html", String(), false, processorSensors);
      });
    server->on("/status", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        pageName="status";
        request->send(FILESYSTEM, "/templateSTA.html", String(), false, processorTemplate);
      });
    server->on("/status.html", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        request->send(FILESYSTEM, "/status.html", String(), false, processorSensors);
      });      
    server->on("/files", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        pageName="files";
        request->send(FILESYSTEM, "/templateSTA.html", String(), false, processorTemplate);
      });
    server->on("/files.html", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
      request->send(FILESYSTEM, "/files.html", String(), false, processorFiles);
      });
    server->on("/getFileList", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        debuglnDated(F("The server received a request 'getFileList'"));
        String fileList=getFileList();
        request->send(200, "text/plain", fileList);
        debuglnDated(F("The server returned file list"));
      });
    // Réception d'une commande de type /getFileContent?fileName=MDS_01.txt
    server->on("/getFileContent", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        debugln(F("The server received a request 'getFileContent'"));
        int paramsNr = request->params();
        Serial.print("Avec ");
        Serial.print(paramsNr);
        debugln(" paramêtre(s)");
        String fileName;
        String fileContent="Rien...";
        for(int i=0;i<paramsNr;i++)
          {
            AsyncWebParameter* p = request->getParam(i);
            if(p->name()=="fileName")
              {
                fileName=p->value();
                debugln("Fichier demandé: " + fileName);
                fileContent = getFileContent(fileName); 
              }
          }

          request->send(200, "text/plain", fileContent);
          debugln("Contenu envoyé!");
      });
    server->on("/saveFile", HTTP_POST,[](AsyncWebServerRequest *request) {
      debugln("Request saveFile received");
      //char fpn[80];
      String fpn;
      // char *ptr = fpn;
      char fileContent[2000];
      if (request->hasParam("filePathName", true)) 
        {
          // * ptr++='/';
          fpn="/" + request->getParam("filePathName", true)->value();
          // strcpy(ptr,request->getParam("filePathName", true)->value().c_str());
          debugln(fpn);
        }
      if (request->hasParam("fileText", true)) 
        {


          strcpy(fileContent,request->getParam("fileText", true)->value().c_str());
          request->send(200, "text/plain", "Text saved...");
        }
          File fileToSave;
          if (fileToSave)
              {
                fileToSave.close();
              }
          debugln("Opening fileToSave in write mode");
          fileToSave = LittleFS.open(fpn, "w"); 
          fileToSave.printf("%s",fileContent);
          fileToSave.flush();
          fileToSave.close();
    });
    server->on("/getListSensors", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        debuglnDated(F("The server received a request 'getListSensors'"));
        String listSensors=getListSensors(); //"000206,07:25,1,21:21,0";
        request->send(200, "text/plain", listSensors);
        // debuglnDated(F("The server returned file list"));
      });
    server->on("/getSensorsStatus", HTTP_GET, [](AsyncWebServerRequest *request)
      {
        // debuglnDated(F("The server received a request 'getSensorsStatus'"));
        String sensorsStatus=getSensorsStatus(); //"000206,07:25,1,21:21,0";
        request->send(200, "text/plain", sensorsStatus);
        // debuglnDated(F("The server returned file list"));
      });    
      server->on("/deleteSensor", HTTP_POST, [](AsyncWebServerRequest *request) 
        {
          debuglnDated(F("The server received a request 'deleteSensor'"));
          String mac;
          if (request->hasParam("mac", true))
            {
              mac = request->getParam("mac", true)->value();
              debugln(mac);
              // int i = getIndexDoorSensor(mac);
              // doorSensor[i].deleteMe();

              getDoorSensor(mac)->deleteMe();
            }
              
        });


      server->on("/updateSensorsList", HTTP_POST, [](AsyncWebServerRequest *request) 
        {
          debugln("Updating sensor list...");
          // bool commonDataChange = false;
          String sensorsList;
          if (request->hasParam("sensorsList", true))
            {
              sensorsList = request->getParam("sensorsList", true)->value();
              debugln(sensorsList);
              int indexLine=0;
              // int indexSensor=0;
              while (true)
                {
                  int newIndexLine=sensorsList.indexOf("$$",indexLine);
                  if (newIndexLine==-1){break;}
                  debugln(newIndexLine);
                  String line=sensorsList.substring(indexLine,newIndexLine);
                  int indexSep1=line.indexOf(",");
                  int indexSep2=line.indexOf(",",indexSep1+1);
                  String mac=line.substring(0,indexSep1);
                  String name=line.substring(indexSep1+1,indexSep2);
                  String description=line.substring(indexSep2+1);
                  debugf4("mac=%s, name=%s, description=%s\n",mac.c_str(),name.c_str(),description.c_str());
                  getDoorSensor(mac)->name=name;
                  getDoorSensor(mac)->description=description;
                  // doorSensor[indexSensor].mac=mac;
                  // doorSensor[indexSensor].name=name;
                  // doorSensor[indexSensor].description=description;
                  colSensors.setValue(mac,name,description,true);
                  // 
                  // indexSensor++;
                  indexLine=newIndexLine+2;  

                }
              // // if(commonData.setValue("mainSSID", mainSSID, "Main SSID")){commonDataChange=true;};
            }
        });
      server->on("/updateCredentials", HTTP_POST, [](AsyncWebServerRequest *request) 
        {
          printlnDated("Updating credentials...");
          // String espName;
          String mainSSID;
          String mainPSW;
          // String fallbackSSID;
          // String fallbackPSW;
          String staGateway;
          String staSubnet;
          String staDNS;
          String staIp;
          // String gmtOffset;
          // String dst;
          bool commonDataChange = false;
          // bool deviceDataChange = false;
          if (request->hasParam("mainSSID", true))
            {
              mainSSID = request->getParam("mainSSID", true)->value();
              if(commonData.setValue("STA_SSID", mainSSID, "Main SSID")){commonDataChange=true;};
            }
          if (request->hasParam("mainPSW", true))
            {
              mainPSW = request->getParam("mainPSW", true)->value();
              if(commonData.setValue("STA_PSW", mainPSW, "Psw main SSID")){commonDataChange=true;};
            }
          // if (request->hasParam("fallbackSSID", true))
          //   {
          //     fallbackSSID = request->getParam("fallbackSSID", true)->value();
          //     if(commonData.setValue("fallbackSSID", fallbackSSID, "SSID secondaire")){commonDataChange=true;};
          //   }
          // if (request->hasParam("fallbackPSW", true))
          //   {
          //     fallbackPSW = request->getParam("fallbackPSW", true)->value();
          //     if(commonData.setValue("fallbackPSW", fallbackPSW, "Psw SSID secondaire")){commonDataChange=true;};
          //   }
          // if (request->hasParam("espName", true))
          //   {
          //     espName = request->getParam("espName", true)->value();
          //     if(deviceData.setValue("espName", espName, "ESP name")){deviceDataChange=true;};
          //     if(commonData.setValue("AP_ssid", "ESP-" + espName, "ESP name on the network")){commonDataChange=true;};
          //   }
          if (request->hasParam("staIp", true))
            {
              staIp = request->getParam("staIp", true)->value();
              if(commonData.setValue("staIp", staIp, "Adresse Ip souhaitée")){commonDataChange=true;};
            }
          if (request->hasParam("staSubnet", true))
            {
              staSubnet = request->getParam("staSubnet", true)->value();
              if(commonData.setValue("staSubnet", staSubnet, "Subnet en mode STA")){commonDataChange=true;};
            }
          if (request->hasParam("staGateway", true))
            {
              staGateway = request->getParam("staGateway", true)->value();
              if(commonData.setValue("staGateway", staGateway, "Gateway en mode STA")){commonDataChange=true;};
            }
          if (request->hasParam("staDNS", true))
            {
              staDNS = request->getParam("staDNS", true)->value();
              if(commonData.setValue("staDNS", staDNS, "DNS en mode STA")){commonDataChange=true;};
            }
          // if (request->hasParam("gmtOffset", true))
          //   {
          //     gmtOffset = request->getParam("gmtOffset", true)->value();
          //     if(commonData.setValue("gmtOffset", gmtOffset, "Décalage GMT")){commonDataChange=true;};
          //   }
          // if (request->hasParam("dst", true))
          //   {
          //     dst = request->getParam("dst", true)->value();
          //     if(commonData.setValue("dst", dst, "Décalage horaire heure d'été")){commonDataChange=true;};
          //   }
          if (commonDataChange){commonData.save();}
          // if (deviceDataChange){deviceData.save();}
        });
      server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) 
        {
          request->send(200, "text/plain", "ESP will restart. Return to home page!");
          //EEPROM.write(0,reg_b);
          //SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
          rebootRequested=true;// ESP.restart();
        });
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
      {
        pageName="info";
        debugln("Chargement page Info...");
        request->send(FILESYSTEM, "/templateSTA.html", String(), false, processorTemplate);
      });
    server->begin();
  }

/* #endregion  Fonctions du serveur*/

/**
 * @brief Prints on the serial console a text with a time prefix. No carriage return at end of line
 * @param text texte to be displayed
 */
void printDated(String text)
  {
    debug(myClock->getNow_YYYY_MM_DD_HH_MM_SS() + " " +  text);
  }
/**
 * @brief Prints on the serial console a text with a time prefix.Carriage return at end of line
 * 
 * @param text texte to be displayed
 */
void printlnDated(String text)
  {
    // debugln(text);
    debugln(myClock->getNow_YYYY_MM_DD_HH_MM_SS() + " " +  text);
  }
bool setupClock()
  {
    if(staModeOK)
      {
        debugln("staMode is OK, starting otp time...");
        timeClient.begin();  // Initialize a NTPClient to get time
        timeClient.setTimeOffset(commonData.getValue("GMT_offset").toInt()*3600);// To be read from settingsto  adjust for your timezone
        return setClockFromOTP();
      }
    return false;
  }
bool setClockFromOTP()
    {
      debugln("Trying to set myClock");
      if(staModeOK)
        {
          timeClient.update();
          time_t epochTime = timeClient.getEpochTime();
          myClock->setClockFromUC(epochTime);
          debuglnDated(F("Clock has been updated from OTP server"));
          return true;
        }
      else 
        {
          debuglnDated(F("Clock update not done because STA is off"));
          return false;
        }
    }
void maintainClock()
  {
    unsigned long period;
    if (myClock->lastTimeUpdate < EPOCH_1_1_2019)
      { // If yes, the clock in not correct!
        period = 60000; // Check each min.
      }
    else
      {
        period = 3600000; // Otherwise, check each h.
      }
    if (currentMillis > lastTimeRequestMillis + period)
      {
        setClockFromOTP();
        lastTimeRequestMillis=millis();
      }
  }
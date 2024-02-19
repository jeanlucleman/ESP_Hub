#ifndef DOORSENSOR_H
  #define DOORSENSOR_H
  #include <Arduino.h>
  #define NBMAXDOORSENSOR 64
  enum enmSwitchState{ // A blind schedule can be disabled, enabled with the same hour each day or enabled by sunrise, sunset time changing automatically each day
    open='0',
    closed='1',
    undefined='2',
  } ;
  class DoorSensor
    {      
      private:
        // unsigned int _macAddress; // Adress mac of ESP-01S of the door sensor.
        // unsigned int _Name;
        uint8_t _state;

      public:    
        uint8_t getState();
        bool setState(uint8_t newState); // return true if chaned
        static size_t doorSensorCount;   
        String name;
        // uint8_t index;
        String mac;
        String description;
        String firmware;
        void deleteMe();
        bool valid; // when a sensor is no more used, this status become false. This will prevent to maintain this sensor in the settings file (sensorList.txt file)
        // bool stateClose;
        DoorSensor();
        void setDoorSensor(String mac);   
    };
      extern DoorSensor doorSensor[]; // array of doorSensor object (the argument in bracket is not necessay as it is defined in the smartSiwtch.cpp)
      // DoorSensor getDoorSensor(String mac);
      // int getIndexDoorSensor(String mac);
      DoorSensor *getDoorSensor(String mac);         

#endif
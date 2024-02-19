#include "doorSensor.h"
#include "global.h"
/* #region --------------------------- D E C L A R A T I O N S -------------------------------------------------*/
  size_t DoorSensor::doorSensorCount=0;// Static variable counting the number of doorSensors
  DoorSensor  doorSensor[NBMAXDOORSENSOR]; // array of pointer on the doorSensor object. Here the size of array must ge given
/* #endregion DECLARATION*/
/* #region ------------------------  D O O R   S E N S O R     C L A S S ---------------------------------------*/
  /**
   * @brief Construct a new DoorSensor:: doorSensor object
   * 
   */
  DoorSensor::DoorSensor() /*Empty constructor of the doorSensor class. Properties of a doorSensor are setup by the setBling procedure*/
    {
      debugln("New object: ");
      _state = enmSwitchState::undefined;
    }
  /**
   * @brief This function is called by the static function initDoorSensorFileSystem at boot to read the doorSensor data in the doorSensorsSettings.ini file and population the class data.
   * 
   * @param _name Name of the doorSensor. This name will be the one displayed on the interface web pages
   * @param _state
   */
  void DoorSensor::setDoorSensor(String _mac)
    {
      // name=_name; // To store as a member variable
      // state=_state;
      debugln("Entrée dans setDoorSensor");
      mac=_mac;
      // index = doorSensorCount; // index of this doorSensor
      doorSensorCount++; // counter of doorSensors (static variable)
      valid=true;
      // checking if a record exists in the file list of door sensor and create it if not existing
      if(colSensors.contains(mac))
        {
          name=colSensors.getValue(mac);
          description=colSensors.getDescription(mac);
        }
      else
        {
          description="undefined";
          name="undefined";
          colSensors.setValue(mac,name,description,true);        
        }
      Serial.printf("New object created in setDoorSensor with mac = %s (%s)\n",mac.c_str(),description.c_str());  
    }
  bool DoorSensor::setState(uint8_t newState)
    {
      if(newState==_state)
        {
          return false;
        }
      else
        {
          _state=newState;
          return true;
        }
    }
  void DoorSensor::deleteMe()
    {
      valid=false;
      colSensors.deleteMe(mac);
    }
  uint8_t DoorSensor::getState()
    {
      return _state;
    }
/*  #endregion Doorsensor class*/
  DoorSensor * getDoorSensor(String mac)
  {
      for (size_t i = 0; i < DoorSensor::doorSensorCount; i++)
        {
          if(doorSensor[i].mac==mac and doorSensor[i].valid==true)
            {
              // Serial.printf("Le switch d'adresse %s (%s) a été récupéré pour l'objet n° %d",mac.c_str(),doorSensor[i].description.c_str(),i);
              // Serial.printf(" et le state en mémoire est %d\n", doorSensor[i].getState());
              return &doorSensor[i];
            }
        }
      // A new record has to be created because there is no switch with this mac value in the array
      size_t i= DoorSensor::doorSensorCount;
      doorSensor[i]=DoorSensor();
      doorSensor[i].setDoorSensor(mac);
      return &doorSensor[i];
  }
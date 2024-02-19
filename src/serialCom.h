/*************************************************************************************************/
/*   This software is made to ease serial wire transmission of String between 2 units.           */                                                              
/*   The main software shall contains the following                                              */
/*     #include "serialCom.h"                                                                    */ 
/*     String ESPName="xxx"; -> the receiver will know from which device it receives a string    */ 
/*     const uint8_t rxPin = 4; // D2 // -> the rx pin of the sender                             */ 
/*     const uint8_t txPin = 5; // D1 // -> the tx pin of the sender                             */ 
/*     int baudRate=38400; -> the baud rate of the sender (the same to be used on the receiver)  */ 
/*     SerialCom* myCom=SerialCom::getInstance(ESPName,baudRate,rxPin,txPin); -> the objet       */ 
/*           to be used to communicate                                                           */ 
/*   The loop must contain   'myCom->readString();'                                              */
/*   Usage:                                                                                      */
/*    myCom->writeString(thisString, ack)                                                        */
/*       If ack is true, the function reports true if the message has been received              */
/*    myCom->dataAvailable()                                                                     */
/*       It returns true if there are string ready to be read                                    */
/*    The messages sent are added to a stack that can be popped and retrieved by                 */
/*    typeIncoming thisMessage=myCom->getData();                                                 */
/*    Jean-Luc LEMAN 01/2024                                                                     */
/*************************************************************************************************/

#ifndef SERIALCOM_H 
  #define SERIALCOM_H
#include <Arduino.h>
// #include <SoftwareSerial.h>
struct typeIncoming
  {
    char sender[10];
    char key[30];
    char content[30];  
  };
class SerialCom
    {
      public:
        static SerialCom* getInstance(String myName, int baudRate, uint8_t rxPin, uint8_t txPin); // Class method as the only access to the Singleton Object
        // void begin(String myName, uint8_t txPin, uint8_t rxPin) ;
        void doStuff();
        bool writeString(String myString, bool ackRequested);
        void readString();
        bool dataAvailable();
        typeIncoming getData();
      private:
        static SerialCom* pInstance; // Static variable holding the pointer to the only instance of this
        SerialCom();
        String _myName;

    };

#endif
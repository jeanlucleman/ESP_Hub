#include "serialCom.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <queue> // https://www.boardinfinity.com/blog/working-of-queue-using-stl/
using namespace std; // Line needed to use the queue include
// struct typeIncoming
//   {
//     char sender[10];
//     char key[30];
//     char content[30];  
//   };
const uint8_t rxPin = 4; // D2
const uint8_t txPin = 5; // D1
SoftwareSerial mySerial (rxPin, txPin);  // Set up a new SoftwareSerial object
queue<typeIncoming> queueIncoming; 
// SoftwareSerial mySerial;

SerialCom* SerialCom::pInstance;
SerialCom* SerialCom::getInstance(String myName, int baudRate, uint8_t rxPin, uint8_t txPin){
	if (pInstance == NULL)
    {
      // Serial.println("Création d'une nouvelle instance de SerialCom");
		  pInstance = new SerialCom();
      pInstance->_myName=myName;
      // SoftwareSerial mySerial (rxPin, txPin);  // Set up a new SoftwareSerial object
      mySerial.begin(baudRate);


    }
  else
    {
      // Serial.println("Instance de SerialCom déjà existante");
    }
	return pInstance;
}

SerialCom::SerialCom(){
  // Serial.begin(115200);
  // delay(1000);
  // Serial.println(F("Initialisation dans SerialCom:SerialCom"));
}
// void SerialCom::setDst(int dst){
//   _dayLightOffset=dst * 3600;
// }





void SerialCom::doStuff() {


}

bool SerialCom::writeString(String myString, bool ackRequested)
  {
    char bufOutgoing[30];
    bool success;
    int counter=0;
    String outcomingString=_myName+";"+myString;
    // Serial.printf("At %lu, string to be sent = %s\n",millis(),outcomingString.c_str());
    if(ackRequested)
      {
        outcomingString= char(1) +outcomingString;
        success=false;
      }
    int len=outcomingString.length()+1;
    // Serial.printf("len= %d\n",len);
    outcomingString.toCharArray(bufOutgoing,len);
    bufOutgoing[len-1]='\n';
    bufOutgoing[len]='\0';
    mySerial.print(bufOutgoing);
    if(ackRequested)
      {
        bool done=false;
        char bufAck[30];
        counter=0;
        bool timeOut=false;
        unsigned long millisStart=millis();
        while(!done and !timeOut)
          {
            char c=mySerial.read();
            if(millis()>millisStart+500)
              {
                timeOut=true;
                success=false;
              }
            yield();
            if(c!=255)
              {
                if(c!=10 and counter<30)
                  {
                    bufAck[counter]=c;
                    counter++;
                  }
                else
                  { 
                    bufAck[counter]='\0';
                    done=true;
                    // Serial.printf("ACK: %s\n",bufAck);           
                    if(strcmp(bufAck,bufOutgoing) != 0)
                      {
                        success=true;
                        // Serial.println("Success!");
                      }
                  }
              }

          }
      }
    // delay(100);
    return success;
  }
void SerialCom::readString()
  {
    char incomingData[50];
    int counter=0;
    bool ackRequest=false;
    char* senderName;
    char* content;
    char* key;
    if(mySerial.available()==0)
    {
      return;
    }
    unsigned long startTime=millis();
    while(true)
      {

 
        // if(c==255){break;}
        if(mySerial.available()>0 and millis()<startTime+1000)
          {
            char c=mySerial.read();
            if(c!=10 and counter <30)
              {
                if(c==1)              // This means an acknowledgement request from the sender
                  {
                    ackRequest=true;
                  }
                else
                  {
                    // readingMode=true;
                    // Serial.printf("%d - ",c);
                    incomingData[counter]=c;
                    counter++;
                    // delay(10);
                  }
              }
            else      // a \n has been received meaining that the whole string has been now reveived
              {
                incomingData[counter]='\0'; // a null char is added at the end to close the string
                // readingMode=false;
                // Serial.println(incomingData);
                char tempBuffer[30];  // This temporary buffer is used to split the string
                strcpy(tempBuffer,incomingData);  

                senderName =strtok(tempBuffer,";"); // The 1st string is the name of sender           
                key = strtok(NULL,";"); // The second is the key 
                content = strtok(NULL,";"); // The third is the content

                // Serial.print("Sender:"); 
                // Serial.println(senderName);
                // Serial.print("Content: ");
                // Serial.println(content);
                typeIncoming message;
                strcpy(message.content,content);
                strcpy(message.sender,senderName);
                strcpy(message.key,key);
                queueIncoming.push(message);
                counter=0 ;                     // Old data in incomingData will be overwritten when new input
                if(ackRequest)  // The sender has requested an acknowledgment
                  {
                    // Serial.printf("%s\n",incomingData);
                    mySerial.printf("%s\n",incomingData);
                    ackRequest=false;
                  }  
                break;             
              }
          }
      }
  }
  bool SerialCom::dataAvailable()
    {
      return !queueIncoming.empty();
    }
  typeIncoming SerialCom::getData()
    {
      typeIncoming thisMessage;
      if( !queueIncoming.empty())
        {
          thisMessage= queueIncoming.front(); // The first element of the queue is read
          queueIncoming.pop(); 
         }
      return thisMessage;
    }
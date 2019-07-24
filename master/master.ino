/* **********************************************************************
 * FileName     : master.ino
 * Date         : 2019.07.15
 * Author       : JaeHong-Min
 * **********************************************************************/



/* INCLUDES *********************************************************** */

#include <Wire.h>


/* DEFINES ************************************************************ */

/* Serial */

#define BAUDRATE 115200
#define ADDRESS  8


/* STRUCTURES ********************************************************* */

struct _Protocol {
  char Command;
  byte Message;
} Protocol;


/* FUNCTIONS ********************************************************** */

void setup() {
  Serial.begin(BAUDRATE);
  
  Wire.begin();
}

void loop() {
  if (Serial.available()) {
    Protocol.Command = Serial.read();
    
    if (Protocol.Command == 'T' || Protocol.Command == 'F') {
      Protocol.Message = Serial.parseInt();
    } else {
      Protocol.Message = 0;
    }
    
    sendProtocol();
    
    if (Protocol.Command == 'C' || Protocol.Command == 'S') {
      //recvProtocol();
      printProtocol();
    }
  }
}

void sendProtocol() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(Protocol.Command);
  Wire.write(Protocol.Message);
  Wire.endTransmission();
}

void recvProtocol() {
  int len = sizeof(Protocol);
  byte data[len];
  
  int cnt = 0;
  
  Wire.requestFrom(ADDRESS, len);
  
  while (cnt < len) {
    if (Wire.available() > 0) {
      data[cnt++] = Wire.read();
    } else {
      delay(10);
    }
  }
  
  // Check request command.
  if (Protocol.Command != data[0]) {
    Serial.println("Protocol Error!");
  }
  
  memcpy(&Protocol, data, len);
}

void printProtocol() {
  Serial.print(Protocol.Command);
  Serial.print(":");
  Serial.println(Protocol.Message);
}

/* ******************************************************************** */

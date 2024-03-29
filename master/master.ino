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

#define AUTO


/* STRUCTURES ********************************************************* */

struct _Protocol {
  char Command;
  byte Data1;
  byte Data2;
} Protocol;


/* FUNCTIONS ********************************************************** */

void setup() {
  Serial.begin(BAUDRATE);
  
  while (!Serial);
  
  Wire.begin();
}

void loop() {
  if (Serial.available()) {
    Protocol.Command = Serial.read();
    
    if (Protocol.Command == 'T' || Protocol.Command == 'F') {
      Protocol.Data1 = Serial.parseInt();
      Protocol.Data2 = 0;
    } else {
      Protocol.Data1 = 0;
      Protocol.Data2 = 0;
    }
    
    sendProtocol();
    
    if (Protocol.Command == 'C' || Protocol.Command == 'S') {
      recvProtocol();
      printProtocol();
    }
  }
}

void sendProtocol() {
  Wire.beginTransmission(ADDRESS);
  Wire.write(Protocol.Command);
  Wire.write(Protocol.Data1);
  Wire.write(Protocol.Data2);
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
  Serial.println(String(Protocol.Command) + ":" + String(Protocol.Data1) + ", " + String(Protocol.Data2));
}

/* ******************************************************************** */

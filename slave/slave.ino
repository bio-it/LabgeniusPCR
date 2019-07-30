/* **********************************************************************
 * FileName     : slave.ino
 * Date         : 2019.07.15
 * Author       : JaeHong-Min
 * **********************************************************************/



/* INCLUDES *********************************************************** */

#include <Wire.h>


/* DEFINES ************************************************************ */

/* Serial */

#define BAUDRATE          115200
#define ADDRESS           8

/* I/O Pins */

#define PIN_FAN           A3
#define PIN_HEATER        A4
#define PIN_THERMISTOR    A0

/* PCR */

#define STATUS_READY      0
#define STATUS_RUN        1
#define STATUS_ERR        2

#define PROCESS_PERIOD    50000 // 50ms

#define a0                1.131786e-003
#define a1                2.336422e-004
#define a3                8.985024e-008
#define RREF              1800
#define T0                273.15
#define dt                PROCESS_PERIOD / 1000000.0f

#define KI_MAX            2600.0f

#define OVERHEAT          150

#define ARRIVAL_DELTA     0.5f


/* STRUCTURES ********************************************************* */

struct _Protocol {
  char Command;
  byte Message;
} Protocol;


/* FIELDS ************************************************************* */

// start, target, kp, ki, kd
float PID_SET[5][5] =
{
  {25, 95, 400, 0.2 , 3000},
  {95, 60, 250, 0.3 , 1000},
  {60, 72, 350, 0.11, 3000},
  {72, 95, 460, 0.18, 3000},
  {95, 50, 500, 0.3 , 1000}
};

byte Status       = STATUS_READY;

float Raw_Temper  = 0.0f;
float Temper      = 0.0f;

float KP          = 0.0f;
float KI          = 0.0f;
float KD          = 0.0f;

float Err         = 0.0f;
float Err_Sum     = 0.0f;
float Err_Pre     = 0.0f;
float PID         = 0.0f;

float preTarget   = 0.0f;
float curTarget   = 0.0f;

byte Fan          = 0;

boolean freeRunning     = false;
boolean targetTempFlag  = false;
boolean isTargetArrival = false;
int freeRunningCounter  = 0;



/* FUNCTIONS ********************************************************** */

void setup() {
  Serial.begin(BAUDRATE);
  
  while (!Serial);
  
  Wire.begin(ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);
  pinMode(PIN_THERMISTOR, INPUT);
  
  controlOff();
}

void loop() {
  unsigned long startTime = micros();
  int processTime = PROCESS_PERIOD;
  
  // Read Temperature
  measure();
  Serial.println(Temper);
  
  // Check Overheat
  if (Temper > OVERHEAT) {
    Status = STATUS_ERR;
    reset();
  }
  
  // Refrigerator
  if (Status == STATUS_READY) {
    digitalWrite(PIN_FAN, Fan);
  }
  
  // Temperature PID Control
  if (Status == STATUS_RUN) {
    control();
  }
  
  processTime -= micros() - startTime;
  delayMicroseconds(processTime > 0 ? processTime : 0);
}

void measure() {
  Raw_Temper = analogRead(PIN_THERMISTOR);
  
  const float u = Raw_Temper / 1024.0f;
  const float r = (1/u-1)*RREF;
  const float lnR = log(r);
  const float temp = a0 + a1 * lnR + a3 * pow(lnR, 3);
  const float inv = 1 / temp;
  
  Temper = inv - T0;
}

void control() {
  if (targetTempFlag && !freeRunning) {
    if (Temper <= curTarget) {
      freeRunning = true;
      freeRunningCounter = 0;
      controlOff();
    }
  }
  
  if (freeRunning) {
    freeRunningCounter++;
    
    if (freeRunningCounter >= (3000000 / PROCESS_PERIOD)) {
      targetTempFlag     = false;
      freeRunning        = false;
      isTargetArrival    = true;
      freeRunningCounter = 0;
    }
  }
  
  if (fabs(Temper - curTarget) < ARRIVAL_DELTA && !targetTempFlag) {
    isTargetArrival = true;
  }
  
  if (isTargetArrival || !freeRunning) {
    controlPID();
  }
}

void controlPID() {
  Err = curTarget - Temper;
  Err_Sum = constrain(Err_Sum + Err * dt, -KI_MAX, KI_MAX);
  
  PID = KP * Err + KI * Err_Sum + KD * (Err - Err_Pre);
  
  Err_Pre = Err;
  
  PID = constrain(PID, 0, 255);
  
  analogWrite(PIN_HEATER, PID);
  digitalWrite(PIN_FAN, Err < -2 ? HIGH : LOW);
}

void findPID() {
  double dist = fabs(preTarget - PID_SET[0][0]) + fabs(curTarget - PID_SET[0][1]);
  double temp;
  
  int index = 0;
  
  for (int i = 1; i < 5; i++) {
    temp = fabs(preTarget - PID_SET[i][0]) + fabs(curTarget - PID_SET[i][1]);
    
    if (temp < dist) {
      dist  = temp;
      index = i;
    }
  }
  
  KP = PID_SET[index][2];
  KI = PID_SET[index][3];
  KD = PID_SET[index][4];
}

void requestEvent() {
  if (Protocol.Command == 'C') {
    Protocol.Message = Temper;
  }
  
  if (Protocol.Command == 'S') {
    Protocol.Message = Status;
  }
  
  Wire.write(Protocol.Command);
  Wire.write(Protocol.Message);
}

void receiveEvent(int len) {
  if (Wire.available() > 0) {
    Protocol.Command = Wire.read();
    Protocol.Message = Wire.read();
    
    if (Protocol.Command == 'R') {   // Reset
      Status = STATUS_READY;
      reset();
    }
    
    if (Status != STATUS_ERR) {
      if (Protocol.Command == 'T') { // Temperature
        Status = STATUS_RUN;
        preTarget = curTarget;
        curTarget = Protocol.Message;
        findPID();
        
        targetTempFlag  = preTarget > curTarget;
        isTargetArrival = false;
      }
      
      if (Protocol.Command == 'F') { // Fan
        reset();
        
        Status = STATUS_READY;
        Fan = Protocol.Message;
      }
    }
    
    printProtocol();
  }
}

void reset() {
  Err_Sum             = 0.0f;
  Err_Pre             = 0.0f;
  
  preTarget           = 0.0f;
  curTarget           = 0.0f;

  Fan                 = 0;
  
  freeRunning         = false;
  targetTempFlag      = false;
  isTargetArrival     = false;
  freeRunningCounter  = 0;

  controlOff();
}

void controlOff() {
  digitalWrite(PIN_FAN, LOW);
  analogWrite(PIN_HEATER, 0);
}

void printProtocol() {
  Serial.print(Protocol.Command);
  Serial.print(":");
  Serial.println(Protocol.Message);
}

/* ******************************************************************** */

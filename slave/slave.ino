#include <Wire.h>

#define BAUDRATE 9600
#define ADDRESS  8

#define PIN_FAN        A4
#define PIN_HEATER     A3
#define PIN_THERMISTOR A0

#define a0       1.131786e-003
#define a1       2.336422e-004
#define a3       8.985024e-008
#define RREF     1800
#define T0       273.15
#define dt       0.2

#define KI_MAX   2600.0f

struct _Protocol {
  char Command;
  byte Message;
  char Status[6];
} Protocol;

String Status     = "Ready";

float Raw_Temper  = 0.0f;
float Temper      = 0.0f;

float Target      = 0.0f;

float KP          = 10.0f;
float KI          = 0.2f;
float KD          = 50.0f;

float Err         = 0.0f;
float Err_Sum     = 0.0f;
float Err_Pre     = 0.0f;
float PID         = 0.0f;

float PreTemp = 0.0f;
float CurTemp = 0.0f;

boolean Temp_Flag = false;

void setup() {
  Serial.begin(BAUDRATE);
  
  Wire.begin(ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_HEATER, OUTPUT);
  pinMode(PIN_THERMISTOR, INPUT);
  
  initial();
}

void loop() {
  unsigned long startTime = micros();
  int sleep = 200000; // 200ms
  
// Read Temperature
  measure();
  
// Temperature PID Control
  if (Status == "Run") {
    control();
  }
  
  sleep -= micros() - startTime;
  delayMicroseconds(sleep > 0 ? sleep : 0);
}

void initial() {
  digitalWrite(PIN_FAN, LOW);
  analogWrite(PIN_HEATER, 0);
}

void measure() {
  Raw_Temper = analogRead(PIN_THERMISTOR);
  
  const float u = Raw_Temper / 1024.0;
  const float r = (1/u-1)*RREF;
  const float lnR = log(r);
  const float temp = a0 + a1 * lnR + a3 * pow(lnR, 3);
  const float inv = 1 / temp;
  
  CurTemp = inv - T0;

  if (Temp_Flag)
    CurTemp = 0.1f * CurTemp + 0.9f * PreTemp;
  else
    Temp_Flag = true;
  PreTemp = CurTemp;

  Temper = CurTemp;

  Serial.println(String(Temper));
}

void control() {
  Err = Target - Temper;
  Err_Sum = constrain(Err_Sum + Err * dt, -KI_MAX, KI_MAX);

  PID = KP * Err + KI * Err_Sum + KD * (Err - Err_Pre);

  Err_Pre = Err;

  PID = constrain(PID, 0, 255);

  analogWrite(PIN_HEATER, PID);
  digitalWrite(PIN_FAN, Err < -2 ? HIGH : LOW);
}

void requestEvent() {
  if (Protocol.Command == 'C') {
    Protocol.Message = Temper;
  }
  
  if (Protocol.Command == 'S') {
    Status.toCharArray(Protocol.Status, 6);
  }
  
  Wire.write((byte *) &Protocol, sizeof(Protocol));
}

void receiveEvent(int len) {
  if (Wire.available() > 0) {
    Protocol.Command = Wire.read();
    Protocol.Message = Wire.read();
    
    if (Protocol.Command == 'T') {
      Target = Protocol.Message;
      Status = "Run";
    }
    
    if (Protocol.Command == 'E') {
      Status = "End";
      initial();
    }
    
    printProtocol();
  }
}

void printProtocol() {
  Serial.print(Protocol.Command);
  Serial.print(":");
  Serial.print(Protocol.Message);
  Serial.print(" ");
  Serial.println(Protocol.Status);
}

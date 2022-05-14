#include "RF24.h"

#define DEBUG_FLG 1
#define DEBUG_PRINT(x){if(DEBUG_FLG){Serial.print(x);}}
 
RF24 rf24(7, 8); // CE腳, CSN腳
//RF setup
const byte addr[] = "1Node";
struct MyData {
  int throttle=0;
  int aileron=0;
  int rudder=0;
  int elevator=0;
  int flap=0;
  int fine=0;
};
MyData msg;

//Joystick input
int PLX = A3, VLX=0;//Pin/Value Right X
int PLY = A2, VLY=0;
int PRX = A1, VRX=0;
int PRY = A0, VRY=0;

//Gain
float thrGain = 0.05;

//Calibrate
int calNum=10;
int RXcal=0, RYcal=0, LXcal=0, LYcal=0;

//Interrupt
const byte RintPin = 2, LintPin = 3, RLEDPin = 4, LLEDPin = 5, thrind = 10;
int lastFine = 1, lastFlap = 1, Vthrint = 0;
 
void setup() {
  //RF
  rf24.begin();
  rf24.setChannel(83);       // 設定頻道編號
  rf24.openWritingPipe(addr); // 設定通道位址
  rf24.setPALevel(RF24_PA_MIN);   // 設定廣播功率
  rf24.setDataRate(RF24_250KBPS); // 設定傳輸速率
  rf24.stopListening();       // 停止偵聽；設定成發射模式
  //Serial
  Serial.begin(9600);
  //Interrupt
  pinMode(RintPin, INPUT_PULLUP);
  pinMode(LintPin, INPUT_PULLUP);
  pinMode(RLEDPin, OUTPUT);
  pinMode(LLEDPin, OUTPUT);
  pinMode(thrind, OUTPUT);/*throttle indicator light
  attachInterrupt(0, flap, FALLING);
  attachInterrupt(1, fine, FALLING);*/
  //Calibrate
  for(int i=0; i<calNum; i++){
      RXcal+=analogRead(PRX);
      RYcal+=analogRead(PRY);
      LXcal+=analogRead(PLX);
      LYcal+=analogRead(PLY);
  }
  RXcal/=calNum;
  RYcal/=calNum;
  LXcal/=calNum;
  LYcal/=calNum;
}
 
void loop() {
  rf24.write(&msg, sizeof(msg));  // 傳送資料
  VRX = analogRead(PRX);  // read the input pin, thr
  VRY = analogRead(PRY);  // read the input pin, rud
  VLX = analogRead(PLX);  // read the input pin, ail
  VLY = analogRead(PLY);  // read the input pin, elv
  DEBUG_PRINT("VRX: ")
  DEBUG_PRINT(VRX)
  DEBUG_PRINT(" ,VRY: ")
  DEBUG_PRINT(VRY)
  DEBUG_PRINT(" ,VLX: ")
  DEBUG_PRINT(VLX)
  DEBUG_PRINT(" ,VLY: ")
  DEBUG_PRINT(VLY)
  DEBUG_PRINT(" ,Vthrint: ")
  DEBUG_PRINT(Vthrint)
  DEBUG_PRINT("\r\n")
  
  msg.throttle+=(VLX-LXcal)*thrGain;
  if(msg.throttle>1000){msg.throttle=1000;}
  else if(msg.throttle<-100){msg.throttle=-100;}
  
  if(msg.throttle>0){
    Vthrint = map(msg.throttle, 0, 1000, 0, 255);
    analogWrite(thrind, Vthrint);
  }
  else{
    Vthrint = 0;
    analogWrite(thrind, 0);
  }
  msg.rudder=-VLY+LYcal;
  msg.aileron=VRY-RYcal;
  msg.elevator=-VRX+RXcal;

  int fineval = digitalRead(LintPin);
  int flapval = digitalRead(RintPin);
  if((lastFine == 1)&&(fineval == 0)){
    fine();
  }
  if((lastFlap == 1)&&(flapval == 0)){
    flap();
  }
  lastFine = fineval;lastFlap = flapval;
  //delay(50);
}

void fine(){
  msg.fine++;
  msg.fine = msg.fine%2;
  digitalWrite(LLEDPin, msg.fine);
  Serial.println("Fine");
}

void flap(){
  msg.flap++;
  msg.flap = msg.flap%2;
  digitalWrite(RLEDPin, msg.flap);
  Serial.println("Flap");
}

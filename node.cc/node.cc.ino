#include <DHT.h>

// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// Monitor / Client NODE

// TODO Count Delays

// CURRENT SIGNALS

// s0-s8 Servos Actuate
// st - 115,116,0,READING
// sh - 115,108,0,READING
// sf - 115,106,0 TODO
// c00 - ack recieved
// c01 - ack send

#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <Adafruit_PWMServoDriver.h>

// Radio -------------------------------------------------
RH_RF95 rf95(8, 7);
#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1
RHReliableDatagram manager(rf95, CLIENT_ADDRESS);
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Controls ----------------------------------------------
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
uint8_t s1[3] = {115, 49, 0}; //s1
uint8_t s2[3] = {115, 50, 0}; //s2
#define SERVOMIN  150 // tune
#define SERVOMAX  450 // tune
int servoPosition = 1;

// Sensors  ----------------------------------------------
#define DHTTYPE DHT22
DHT dht(18, DHTTYPE);

#define FLOWPIN 19
uint16_t pulses = 0;
uint8_t lastflowpinstate;
uint32_t lastflowratetimer= 0;
float flowrate;


int led =13;

void setup() {
  Serial.begin(9600);
//  while(!Serial);
  pinMode(led, OUTPUT); 
//  if (!rf95.init())   Serial.println("Radio init failed");
  if(!manager.init()) Serial.println("Proto init failed");
  pwm.begin();
  pwm.setPWMFreq(60); 
  dht.begin();

  pinMode(FLOWPIN, INPUT);
  digitalWrite(FLOWPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWPIN);
}

void loop(){


  uint8_t len = sizeof(buf);
  uint8_t from; 

  //Ack Packets
  uint8_t ack[] = "c01";
  manager.sendtoWait(ack, sizeof(ack), SERVER_ADDRESS);
  //End Ack Packets

  //Data Readings
  int humid = dht.readHumidity();
  int temp = dht.readTemperature();
  
  if (isnan(humid) || isnan(temp)) {
   Serial.println("Failed to read from DHT sensor!");
  }
  
  char tdata = temp;
  uint8_t stdata[4] = {115, 116, 0, tdata}; 
  if(manager.sendtoWait(stdata, sizeof(stdata), SERVER_ADDRESS)){
      Serial.print("Temp Sent: ");
      Serial.println(temp);
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else{
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  
  char hdata = humid;
  uint8_t shdata[4] = {115, 108, 0, hdata};
  if(manager.sendtoWait(shdata, sizeof(shdata), SERVER_ADDRESS)){
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
      Serial.print("Humid Sent: ");
      Serial.println(humid);
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else{
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }    
  }

  char fdata = flowRate();
  uint8_t sfdata[4] = {115, 102, 0, fdata};
  if(manager.sendtoWait(sfdata, sizeof(sfdata), SERVER_ADDRESS)){
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
      Serial.print("Flow Sent: ");
      Serial.println(fdata);
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else{
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }    
  }
  //END DATA READINGS 

  if(manager.available()){
  if(manager.recvfromAckTimeout(buf, &len, 3000, &from)){ 
      Serial.print("got reply: ");
      Serial.println((char*)buf);
      signalBlink(3); 
      if(buf[0]==s1[0] && buf[1]==s1[1] && servoPosition==2){
        for(uint16_t i = SERVOMIN; i < SERVOMAX; i++){
          pwm.setPWM(0,0,i);
        }
          servoPosition = 1;
          uint8_t data[] = "s001";
          manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS);
      }
      if(buf[0]==s2[0] && buf[1]==s2[1] && servoPosition==1){
        for(uint16_t i = SERVOMAX; i > SERVOMIN; i--){
          pwm.setPWM(0,0,i);
        } 
          servoPosition = 2;       
          uint8_t data[] = "s002";
          manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS);
      }
    }
  }
  
  delay(100); //for viewing purposes
}// end loop


int flowRate(){
  uint8_t x = digitalRead(FLOWPIN);
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz 
  lastflowratetimer = 0;
  return flowRate;
}

void errorBlink(){
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
    delay(100);
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
    delay(100);
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
}

void signalBlink(int count){
  for(int i = 0; i<count;i++){
    digitalWrite(led,HIGH);
    delay(200);
    digitalWrite(led,LOW);
    delay(50);
  }
}

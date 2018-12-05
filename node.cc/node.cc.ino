#include <DHT.h>

// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// Monitor / Client NODE


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
#define SERVOMIN  100 // tune
#define SERVOMAX  400 // tune
int servoPosition = 1;

// Sensors  ----------------------------------------------
#define DHTTYPE DHT22
DHT dht(18, DHTTYPE);
int lasthumid = 0;
int lasttemp = 0;


#define FLOWPIN 19
uint16_t pulses = 0;
uint8_t lastflowpinstate;
uint32_t lastflowratetimer= 0;
float flowrate;


int led =13;

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT); 
//  if (!rf95.init())   Serial.println("Radio init failed");
  if(!manager.init()) Serial.println("Proto init failed");
  pwm.begin();
  pwm.setPWMFreq(40); 
  dht.begin();

  pinMode(FLOWPIN, INPUT);
  digitalWrite(FLOWPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWPIN);
}

void loop(){


  uint8_t len = sizeof(buf);
  uint8_t from; 

  //Ack Packets
  uint8_t ack[] = {99,99,48,49};
  if(manager.sendtoWait(ack, sizeof(ack), SERVER_ADDRESS)){
      Serial.print("Sent Ack: ");
      Serial.print((char) ack[0]);
      Serial.print((char) ack[1]);
      Serial.print((char) ack[2]);
      Serial.println((char) ack[3]);
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      controls(buf);
    }
  }
  //End Ack Packets

  //Data Readings
  int humid = dht.readHumidity();
  int temp = dht.readTemperature();
 
  if (isnan(humid) || isnan(temp)) {
        uint8_t data[] = "cc04"; 
    if(manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS)){
      Serial.print("Bad Reading");
      if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
        Serial.print("got reply from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println((char*)buf);
        controls(buf);
      }
      else{
        Serial.println("No reply, is the server running?");
      }
    }
  }
  
  if(temp != lasttemp){
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
      controls(buf);
    }
    else{
      Serial.println("No reply, is the server running?");
    }
  }
  lasttemp = temp;
  }

  if(humid != lasthumid){
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
      controls(buf);
    }
    else{
      Serial.println("No reply, is the server running?");
    }    
  }
  lasthumid = humid;
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
      controls(buf);
    }
    else{
      Serial.println("No reply, is the server running?");
    }
        
  }
  //END DATA READINGS 
  
  delay(500); //for viewing purposes
}// end loop

void controls(uint8_t buf[]){
    //Controls S0-9
    if(buf[0]==115 && (0<=buf[1]<=10)){
        if(buf[3]==49){
          Serial.println("Servo+");
            for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
              pwm.setPWM(buf[1], 0, pulselen);
            }
        }
        else if(buf[3]==50){
          Serial.println("Servo-");
            for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
              pwm.setPWM(buf[1], 0, pulselen);
            }
        }
        else{
          pwm.setPWM(0,0,map(buf[3], 1,255, SERVOMIN, SERVOMAX)); 
        }
        char dev = buf[1];
        char pos = buf[3];
        //CNXX
        uint8_t data[4] = {99, 110, dev, pos}; 
        if(manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS)){
          Serial.print("Servo");
          Serial.print(buf[1]);
          Serial.print("Moved To");
          Serial.println(buf[3]);
        }
        else{
          Serial.println("No reply, is the server running?");
        }
    }
    if(buf[0]==115 && buf[1]==49 && buf[1]==48 && buf[1]==49){
      setup();
    }
}

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

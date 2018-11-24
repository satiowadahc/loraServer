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
#include <RH_RF95.h>
#include <Adafruit_PWMServoDriver.h>


RH_RF95 rf95(8, 7);

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

uint8_t s1[3] = {115, 49, 0}; //s1
uint8_t s2[3] = {115, 50, 0}; //s2

#define SERVOMIN  150 // tune
#define SERVOMAX  450 // tune
int servoPosition = 1;

#define DHTTYPE DHT22

DHT dht(18, DHTTYPE);

int led =13;
void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT); 
  if (!rf95.init())
    Serial.println("init failed");
  //TODO add in blink code

  //TODO Check for errrs?
  pwm.begin();
  pwm.setPWMFreq(60); 

  dht.begin();
}

void loop() {
  //Ack Packets
  uint8_t ack[] = "c01";
  rf95.send(ack, sizeof(ack));
  rf95.waitPacketSent();
  //End Ack Packets

  //Data Readings
  int humid = dht.readHumidity();
  int temp = dht.readTemperature();
  
  if (isnan(humid) || isnan(temp)) {
   Serial.println("Failed to read from DHT sensor!");
  }
  
  char tdata = temp;
  uint8_t stdata[4] = {115, 116, 0, tdata}; 
  rf95.send(stdata, sizeof(stdata));
  rf95.waitPacketSent();
  
  char hdata = humid;
  uint8_t shdata[4] = {115, 108, 0, hdata};
  rf95.send(shdata, sizeof(shdata));
  rf95.waitPacketSent();
  
  //END DATA READINGS
  
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
      signalBlink(len); 
      if(buf[0]==s1[0] && buf[1]==s1[1] && servoPosition==2){
        for(uint16_t i = SERVOMIN; i < SERVOMAX; i++){
          pwm.setPWM(0,0,i);
        }
          servoPosition = 1;
          uint8_t data[] = "Servo 1 Triggered";
          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();
      }
      if(buf[0]==s2[0] && buf[1]==s2[1] && servoPosition==1){
        for(uint16_t i = SERVOMAX; i > SERVOMIN; i--){
          pwm.setPWM(0,0,i);
        } 
          servoPosition = 2;       
          uint8_t data[] = "Servo 2 Triggered";
          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();
      }
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
    errorBlink();
  }
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

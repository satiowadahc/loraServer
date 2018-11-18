// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// Monitor / Client NODE

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_PWMServoDriver.h>


RH_RF95 rf95(8, 7);

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
uint8_t s1[3] = {115,49,0};
uint8_t s2[3] = {115,50,0};
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  450 // this is the 'maximum' pulse length count (out of 4096)
int servoPosition = 1;

int led =13;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led, OUTPUT); 
  
  //TODO DELETE THIS
//  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");

  pwm.begin();
  pwm.setPWMFreq(60); 
}

void loop() {
  // Send a message to rf95_server
  uint8_t data[] = "Hello World!";
  rf95.send(data, sizeof(data));
  
  rf95.waitPacketSent();
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
  }
  delay(1000);
}

// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// DISPLAY / SERVER NODE

#include <RadioHead.h>0
#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
//#include <Wire.h>

#define TFT_CS   9
#define TFT_DC   10
#define STMPE_CS 6
#define SD_CS    5

#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

#define BOXSIZE 40
#define PENRADIUS 3

RH_RF95 rf95(8, 7);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

int led = 13;
int msgcount = 1;
int select;
TS_Point p, op;
void setup() {
  delay(50);
  pinMode(led, OUTPUT);     
  
  tft.begin();
  tft.setRotation(3);
  tft.setCursor(0, 0); 
  tft.setTextColor(ILI9341_WHITE);  
  tft.setTextSize(1);
  
  if (!rf95.init()){
    tft.println("Radio init failed");
  }
  if (!ts.begin()){
    tft.println("Touch init failed");
  }
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(225, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
  tft.fillRect(225+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
}

void loop() {

  // TOUCH STUFF
  op = p;
  p = ts.getPoint();
  if(p!=op){
    tft.print(p.x);
    tft.print(",");
    tft.println(p.y);
    msgcount++;
  }
  if((0 < p.y) && (p.y < BOXSIZE) && (225 < p.x) && (p.x < 225+BOXSIZE)){
    if(p.x < 265){
      uint8_t data[] = "servo1";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      tft.println("servo1");
    }
    else{
      if(p.x < 265){
        uint8_t data[] = "servo2";
        rf95.send(data, sizeof(data));
        rf95.waitPacketSent();
        tft.println("servo2");
      }
    }
  }

  // Radio stuff
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
      
      tft.print(msgcount);
      tft.print("got request: ");
      tft.println((char*) buf);
      
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();

      tft.println("Sent a reply");
      digitalWrite(led, LOW);
      
    }
    else
    {
      tft.println("recv failed");
    }  
    msgcount++;
  }
  

  if(msgcount%14 ==0 ){
    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect(225, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
    tft.fillRect(225+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
    tft.setCursor(0, 0); 
  }
  delay(100);
}

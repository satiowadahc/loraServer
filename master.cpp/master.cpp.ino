// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// DISPLAY / SERVER NODE

//#include <RadioHead.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Wire.h>

#define TFT_CS   9
#define TFT_DC   10
#define STMPE_CS 6
#define SD_CS    5

#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

#define BOXSIZE 80

RH_RF95 rf95(8, 7);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

int led = 13;
int msgcount = 1;

TS_Point p, op;
uint8_t checker[13] = {72,101,108,108,111,32,87,111,114,108,100,33,0};
void setup() {
  delay(50);
  pinMode(led, OUTPUT);     
  
  tft.begin();

  Serial.begin(9600);
  
  if (!rf95.init()){
    tft.println("Radio init failed");
  }
  if (!ts.begin()){
    tft.println("Touch init failed");
  }  
  
  tft.setRotation(3);
  tft.setCursor(0, 0); 
  tft.setTextColor(ILI9341_RED); 
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(160, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
  tft.fillRect(160+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
}


int ry, rx;
void loop() {

  // TOUCH STUFF
  op = p;
  p = ts.getPoint(); 
  rx = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  ry = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  if(p!=op){
    Serial.print(rx);
    Serial.print(",");
    Serial.println(ry);
    if(0 < rx && rx < BOXSIZE){
      if(ry < BOXSIZE){
          uint8_t data[] = "s2";
          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();
          Serial.println("servo2");
          delay(50);
      }
      if(BOXSIZE < ry && ry < BOXSIZE*2){
        uint8_t data[] = "s1";
        rf95.send(data, sizeof(data));
        rf95.waitPacketSent();
        Serial.println("servo1");
        delay(50);
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
      if(buf[0] == checker[0]){
        tft.fillRect(160,BOXSIZE,BOXSIZE,BOXSIZE,ILI9341_GREEN);
      }
      else if(buf[0] == 115){
        tft.fillRect(160,BOXSIZE,BOXSIZE,BOXSIZE,ILI9341_RED);
        delay(500);
      }
      delay(100);
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      tft.fillRect(160,BOXSIZE,BOXSIZE,BOXSIZE,ILI9341_CYAN);
      delay(50);
      
    }
    else
    {
      tft.println("recv failed");
    }  
  }
  

  if(msgcount%7 ==0 ){
    tft.setCursor(0, 0); 
    msgcount=1;
  }
  delay(100);
  tft.fillRect(160,BOXSIZE,BOXSIZE,BOXSIZE,ILI9341_BLACK);
  
}

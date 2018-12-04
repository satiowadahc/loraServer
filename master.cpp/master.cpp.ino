// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// DISPLAY / SERVER NODE

// improve servo message

//#include <RadioHead.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Wire.h>


// Touch Screen
#define TFT_CS   9
#define TFT_DC   10
#define STMPE_CS 6
#define SD_CS    5
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750
#define BOXSIZE 80
TS_Point p, op;
int ry, rx;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

//Radio
#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1
RH_RF95 rf95(8, 7);
RHReliableDatagram manager(rf95, SERVER_ADDRESS);
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];


////Display info for connection status
bool conFlag = false;
int count = 0;

void setup() {   
  
  tft.begin();
  
  if (!ts.begin())     tft.println("Touch init failed");  
  if (!manager.init()) tft.println("Proto init failed");
  
  tft.setRotation(3);//USB FACING DOWN
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(160, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
  tft.fillRect(160+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
  tft.fillRect(160, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_ORANGE);
  tft.fillRect(160+BOXSIZE, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_BLUE);
  tft.fillRect(160, BOXSIZE*2, BOXSIZE*2, BOXSIZE, ILI9341_GREEN);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.setCursor(180, 25);
  tft.println("T+");
  tft.setCursor(260, 25);
  tft.println("T-");
  tft.setCursor(180, 25+BOXSIZE);
  tft.println("On");
  tft.setCursor(245, 25+BOXSIZE);
  tft.println("Off");
  tft.setCursor(180, 25+BOXSIZE*2);
  tft.println("Pump");   
  tft.setTextColor(ILI9341_BLACK); 
  tft.setTextSize(2);
}

void loop() {

  // PUT ------------------------------------------------------------
  op = p;
  p = ts.getPoint(); 
  rx = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  ry = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  if(p!=op){
    if(0 < rx && rx < BOXSIZE){
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(4);
      if(ry < BOXSIZE){
          tft.fillRect(160+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
          uint8_t data[] = "s2";
          manager.sendtoWait(data, sizeof(data), CLIENT_ADDRESS);
          tft.fillRect(160+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
          tft.setCursor(260, 25);
          tft.println("T-");
          tft.fillRect(0,220,160,30,ILI9341_GREEN);
          tft.setTextColor(ILI9341_BLACK); 
          tft.setTextSize(2);
          tft.setCursor(2,225);
          tft.print("T- Request Sent");
      }
      if(BOXSIZE < ry && ry < BOXSIZE*2){
        tft.fillRect(160, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        uint8_t data[] = "s1";
        manager.sendtoWait(data, sizeof(data), CLIENT_ADDRESS);
        tft.fillRect(160, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
        tft.setCursor(180, 25);
        tft.println("T+");
        tft.fillRect(0,220,160,30,ILI9341_GREEN);
        tft.setTextColor(ILI9341_BLACK); 
        tft.setTextSize(2);
        tft.setCursor(2,225);
        tft.print("T+ Request Sent");
      }
    }
  }

  // Radio stuff
  if (manager.available()){
      uint8_t len = sizeof(buf);
      uint8_t from;
    if (manager.recvfromAck(buf, &len, &from)){
      //GET ----------------------------------------------
      if(buf[0] == 115){
        //st
        if(buf[1] == 116){
          tft.fillRect(0,0,160,25,ILI9341_WHITE);
          tft.setCursor(2,5);
          tft.print("Well Temp: ");
          tft.println(buf[3]);
        } 
        //sh
        else if (buf[1] == 108){
          tft.fillRect(0,25,160,25,ILI9341_WHITE);
          tft.setCursor(2,30);
          tft.print("Humidity: ");
          tft.println(buf[3]);
        }
        //sf
        else if (buf[1] == 102){
          //TODO CONNECT SENSOR
          tft.fillRect(0,50,160,25,ILI9341_WHITE);
          tft.setCursor(2,55);
          tft.print("Flow: ");
          tft.println(buf[3]);
        }
        uint8_t data[] = "cc02";
        manager.sendtoWait(data, sizeof(data), from);
        tft.fillRect(0,220,160,30,ILI9341_GREEN);
        tft.setCursor(2,225);
        tft.print("Data RX");
        conFlag = true;
        count = 0;
      
      }//END DATA SWITCH
      else if(buf[0] == 99 && buf[1] == 99){
        //c01 - Send Connected with no data
        if(buf[2] == 48 && buf[2] == 49){
          tft.fillRect(0,220,160,30,ILI9341_GREEN);
          tft.setCursor(2,225);
          tft.print("Connected");
          conFlag = true;
          count = 0;
        }
      } //End connection switch
      else{
        //403 - Bad request
        uint8_t data[] = "cc03";
        manager.sendtoWait(data, sizeof(data), from);
      }
     }//End Message Readings
   }
   else{
    count++;
   }

  if(count%20 == 1 && !conFlag){
      setup();
      tft.fillRect(0,220,160,30,ILI9341_GREEN);
      tft.setCursor(2,225);
      tft.print("Timer:");
      tft.print(count-1);
  }
  conFlag = false;
  delay(500);
} // end loop

// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// DISPLAY / SERVER NODE

// TODO Count Delays
// Add in signal chart
// improve servo message


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

TS_Point p, op;
int ry, rx;

#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

#define BOXSIZE 80

RH_RF95 rf95(8, 7);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

int led = 13;

//Display info for connection status
bool conFlag = false;
int count = 0;



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

  tft.fillScreen(ILI9341_BLACK);
  //TODO COLOUR SCHEME
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
  
  tft.setTextColor(ILI9341_RED); 
  tft.setTextSize(2);
}

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

  //I Don't think available is the proper function here
  if (rf95.available()){
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    uint8_t tbuf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t tlen = sizeof(buf);
    
    if (rf95.recv(buf, &len)){
      Serial.println("---");
      for(int i = 0; i <20; i++){
        Serial.println(buf[i]);
      }

      //Data Reading
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
          tft.fillRect(0,50,160,25,ILI9341_WHITE);
          tft.setCursor(2,55);
          tft.print("Flow: ");
          tft.println(buf[3]);
        }
      }//End Data Readings

      //Status readings
      //cxx
      if(buf[0] == 99){
        //c01
        if(buf[1] == 48 && buf[2] == 49){
          tft.fillRect(0,220,160,30,ILI9341_GREEN);
          tft.setCursor(2,225);
          tft.print("Connected");
          conFlag = true;
          count = 0;
        }
      }
      // Ack
      uint8_t data[] = "c00";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      
      delay(50);
    }
    else
    {
      tft.println("recv failed");
    }   
  } // End Radios

  delay(100);
  if(conFlag){
    conFlag = false;
  }
  else{
    count++;
    if(count%20==0){
    tft.fillRect(0,225,160,30,ILI9341_GREEN);
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(2,225);
    tft.print(count);
    tft.setTextColor(ILI9341_BLACK);
    }
  }
} // end loop

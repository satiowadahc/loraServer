// CMPT 430 Final Design
// Chad A. Woitas
// Well monitoring project

// DISPLAY / SERVER NODE

#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>


// SPI Chip Select
#define TFT_CS   9
#define TFT_DC   10
#define TS_CS 6
#define RH_CS 8

#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750
#define BOXSIZE 80

TS_Point p, op;
int ry, rx;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(TS_CS);

//Radio
#define CLIENT_ADDRESS 2
#define SERVER_ADDRESS 1
RH_RF95 rf95(8, 7);
RHReliableDatagram manager(rf95, SERVER_ADDRESS);

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
//uint8_t messagetosend[] = "cc02";
uint8_t messageFlag = 0;

//Display info for connection status


void setup() {   
  Serial.begin(9600);
  
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
  tft.println(F("T+"));
  tft.setCursor(260, 25);
  tft.println(F("T-"));
  tft.setCursor(180, 25+BOXSIZE);
  tft.println(F("On"));
  tft.setCursor(245, 25+BOXSIZE);
  tft.println(F("Off"));
  tft.setCursor(180, 25+BOXSIZE*2);
  tft.println(F("Pump"));   
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
      //Square T- -----------------------------
      if(ry < BOXSIZE){
          tft.fillRect(160+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
          messageFlag = 4;
          tft.fillRect(160+BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
          tft.setCursor(260, 25);
          tft.println(F("T-"));
          tft.fillRect(0,220,160,30,ILI9341_GREEN);
          tft.setTextColor(ILI9341_BLACK); 
          tft.setTextSize(2);
          tft.setCursor(2,225);
          tft.print(F("Sent: T-"));
      }
      // Square T+ ----------------------------
      if(BOXSIZE < ry && ry < BOXSIZE*2){
        tft.fillRect(160, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        messageFlag = 3;
        tft.fillRect(160, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
        tft.setCursor(180, 25);
        tft.println(F("T+"));
        tft.fillRect(0,220,160,30,ILI9341_GREEN);
        tft.setTextColor(ILI9341_BLACK); 
        tft.setTextSize(2);
        tft.setCursor(2,225);
        tft.print(F("Sent: T+"));
      }
    }
    else if(BOXSIZE < rx && rx < BOXSIZE*2){
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(4);
      //Off ---------------------------------- 
      if(ry < BOXSIZE){
          tft.fillRect(160+BOXSIZE, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
          tft.fillRect(160+BOXSIZE, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_BLUE);
          tft.setCursor(245, 25+BOXSIZE);
          tft.println(F("Off"));
          tft.fillRect(0,220,160,30,ILI9341_GREEN);
          tft.setTextColor(ILI9341_BLACK); 
          tft.setTextSize(2);
          tft.setCursor(2,225);
          tft.print(F("Sent: Off"));
      }
      //On ----------------------------------
      if(BOXSIZE < ry && ry < BOXSIZE*2){
        tft.fillRect(160, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.fillRect(160, BOXSIZE, BOXSIZE, BOXSIZE, ILI9341_ORANGE);
        tft.setCursor(180, 25+BOXSIZE);
        tft.println(F("On"));
        tft.fillRect(0,220,160,30,ILI9341_GREEN);
        tft.setTextColor(ILI9341_BLACK); 
        tft.setTextSize(2);
        tft.setCursor(2,225);
        tft.print(F("Sent: On"));
      }
    }
    else if(BOXSIZE*2 < rx){
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(4);
      //Pump ---------------------------------- 
      //Using a refresh
      if(ry < BOXSIZE*2){
          tft.fillRect(160, BOXSIZE*2, BOXSIZE*2, BOXSIZE, ILI9341_WHITE);
          messageFlag = "5";
          tft.fillRect(160, BOXSIZE*2, BOXSIZE*2, BOXSIZE, ILI9341_GREEN);
          tft.setCursor(180, 25+BOXSIZE*2);
          tft.println(F("Pump"));
          tft.fillRect(0,220,160,30,ILI9341_GREEN);
          tft.setTextColor(ILI9341_BLACK); 
          tft.setTextSize(2);
          tft.setCursor(2,225);
          tft.print(F("Refreshed"));
          setup();
      }
    }
    //Wait a sec for return packets
    delay(200);
  }

  // Radio stuff
  if (manager.available()){
      uint8_t len = sizeof(buf);
      uint8_t from;
    if (manager.recvfromAck(buf, &len, &from)){
      Serial.println((char*) buf);
      //GET ----------------------------------------------
      //DATA Switch SX00
      if(buf[0] == 115){
        //
        if(buf[1] == 116){
          tft.fillRect(0,0,160,25,ILI9341_WHITE);
          tft.setCursor(2,5);
          tft.print(F("Well Temp: "));
          tft.println(buf[3]);
        } 
        //sh
        else if (buf[1] == 108){
          tft.fillRect(0,25,160,25,ILI9341_WHITE);
          tft.setCursor(2,30);
          tft.print(F("Humidity: "));
          tft.println(buf[3]);
        }
        //sf
        else if (buf[1] == 102){
          tft.fillRect(0,50,160,25,ILI9341_WHITE);
          tft.setCursor(2,55);
          tft.print(F("Flow: "));
          tft.println(buf[3]);
        }
        mesFlag(messageFlag, from);
//        manager.sendtoWait(messagetosend, sizeof(messagetosend), from);
        //reset message
//        uint8_t messagetosend[] = "cc02";
        tft.fillRect(0,220,160,30,ILI9341_GREEN);
        tft.setCursor(2,225);
        tft.print(F("Data RX"));  
      }//END DATA SWITCH
      
      //Connection switch CXXX
      else if(buf[0] == 99){
        if(buf[1] == 99){       
          //CC01
          if(buf[2] == 48 && buf[3] == 49){
            tft.fillRect(0,220,160,30,ILI9341_GREEN);
            tft.setCursor(2,225);
            tft.print(F("Connected"));           
          }
          //TODO add in CC00 -> CC04?
            mesFlag(messageFlag, from);
//          manager.sendtoWait(messagetosend, sizeof(messagetosend), from);
          //reset message
//          uint8_t messagetosend[] = "cc02";
        }
        //CNXX
        else if(buf[1]==110){
            tft.fillRect(0,220,160,30,ILI9341_GREEN);
            tft.setCursor(2,225);
            tft.print("S");
            tft.print(buf[2]);
            tft.print(" >> ");
            tft.print(buf[3]);            
        }
      } //End connection switch
      
      else{
        //403 - Bad request
         mesFlag(1, from);
//        uint8_t data[] = "cc03";
//        manager.sendtoWait(data, sizeof(data), from);
      }
     }//End Message Readings
     

   }//End Manager
   else{
   }


  delay(500);
} // end loop

void mesFlag(int f, uint8_t from){

  if(f==0){
    //CC02
    uint8_t data[] = {99,99,48,50};
    manager.sendtoWait(data, sizeof(data), from);
  } else if(f==1){
    //CC03
    uint8_t data[] = {99,99,48,51};
    manager.sendtoWait(data, sizeof(data), from);
  } else if(f==3){
    //S001
    uint8_t data[] = {115,0,48,49};
    manager.sendtoWait(data, sizeof(data), from);
  } else if(f==4){
    //S002
    uint8_t data[] = {115,0,48,50};
    manager.sendtoWait(data, sizeof(data), from);
  } else if(f==5){
    //S101
    uint8_t data[] = {115,49,48,49};
    manager.sendtoWait(data, sizeof(data), from);
  }  
   
   messageFlag = 0;      
}

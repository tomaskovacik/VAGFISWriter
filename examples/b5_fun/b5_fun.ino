#include "VAGFISWriter.h"
#include "bitmaps.h"

#define MinRefresh 100

// FIS
//stm32
//#define FIS_CLK PB3
//#define FIS_DATA PB5
//#define FIS_ENA PA15

//avr
#define FIS_ENA 5
#define FIS_CLK 6
#define FIS_DATA 7

VAGFISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA );
static char fisBuffer[10] = {'B', '5', ' ', 'F', 'A', 'M', 'I', 'L', 'I', 'A'} ;

void lf() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {left_door[line]};
    fisWriter.GraphicOut(15, line + 19, 1, tmpdata, 1);
  }
}

void lr() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {left_door[line]};
    fisWriter.GraphicOut(15, line + 19 + 8, 1, tmpdata, 1);
  }
}

void rf() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {right_door[line]};
    fisWriter.GraphicOut(41, line + 19, 1, tmpdata, 1);
  }
}
void rr() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {right_door[line]};
    fisWriter.GraphicOut(41, line + 19 + 8, 1, tmpdata, 1);
  }
}

void trunk_avant() {
  fisWriter.GraphicFromArray(25, 41, 14, 4, avant_trunc, 1);
}
void trunk_sedan() {
  fisWriter.GraphicFromArray(23, 38, 18, 7, sedan_trunc, 1);
}

void trunk() {
  trunk_sedan();
}

void setup() {
  fisWriter.begin();
  fisWriter.reset();
  Serial.begin(115200);
}


void loop() {  
  fisWriter.initFullScreen();
  fisWriter.GraphicFromArray(0, 0, 64, 88, b5f, 1);
  delay(3000);
  //inverted
//  fisWriter.initFullScreenFilled();
  //draw mode = 1, only 0 from data are used 
//  fisWriter.GraphicFromArray(0, 0, 64, 88, b5f, 0);
//  delay(3000);
  fisWriter.initFullScreenFilled();
  //draw mode = 1, only 0 from data are used 
  fisWriter.GraphicFromArray(0, 0, 64, 88, b5f, 1);
  delay(1000);
  fisWriter.GraphicFromArray(0, 0, 64, 88, b5f, 2);
  delay(3000);
  fisWriter.initFullScreen();
  fisWriter.GraphicFromArray(0, 0, 64, 65, Q, 2);
  fisWriter.GraphicFromArray(0, 70, 64, 16, QBSW, 2);
  delay(3000);
  fisWriter.reset();
  fisWriter.sendMsg("12345678  TEST  ");
  delay(100);
  fisWriter.initMiddleScreen();
  fisWriter.GraphicFromArray(22, 1, 20, 46, sedan, 2);
  for (uint8_t x = 0; x < 2; x++) { //speed test
    lf();
    lr();
    rf();
    rr();
    trunk();
    lf();
    lr();
    rf();
    rr();
    trunk();
  }
  fisWriter.sendMsg("  TEST  12345678");
  delay(1000);
  fisWriter.initFullScreen();

  fisWriter.sendMsgFS(0,0,0x00,6,"LINE 1"); // 0x00 - standard font, negative, left aligned
  fisWriter.sendMsgFS(0,8,0x01,6,"LINE 2"); // 0x01 - standard font, positive, left aligned
  fisWriter.sendMsgFS(0,16,0x04,6,"LINE 3"); // 0x04 - compressed font, negative, left aligned
  fisWriter.sendMsgFS(0,24,0x05,6,"LINE 4"); // 0x05 - compressed font, positive, left-aligned
  fisWriter.sendMsgFS(0,32,0x08,6,"LINE 5"); // 0x08 - special characters, negative, left aligned
  fisWriter.sendMsgFS(0,40,0x09,6,"LINE 6"); // 0x09 - special characters, positive, left aligned
  fisWriter.sendMsgFS(0,48,0x20,6,"LINE 7"); // 0x20 - standard font, negative, centered
  fisWriter.sendMsgFS(0,56,0x21,6,"LINE 8"); // 0x21 - standard font, positive, centered
  fisWriter.sendMsgFS(0,64,0x24,6,"LINE 9"); // 0x24 - compressed font, negative, centered
  fisWriter.sendMsgFS(0,72,0x25,13,"1234567891011"); // 0x25 - compressed font, positive, centered
  fisWriter.sendMsgFS(0,80,0x28,13,"1234567891011"); // 0x28 - special characters, positive, centered
  delay(5000);
  fisWriter.initFullScreen();
  fisWriter.sendMsgFS(0,0,0x29,7,"LINE 1"); // 0x29 - special characters, negative, centered
  delay(5000);
  //while(true){ //and display will did not change ever
  //  fisWriter.sendKeepAliveMsg();
  //  delay(1000);
  //}
}

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
uint8_t frameBuffer[704];

void lf() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {left_door[line]};
    fisWriter.GraphicOut(15, line + 19, 1, tmpdata, 1, 0);
  }
}

void lr() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {left_door[line]};
    fisWriter.GraphicOut(15, line + 19 + 8, 1, tmpdata, 1, 0);
  }
}

void rf() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {right_door[line]};
    fisWriter.GraphicOut(41, line + 19, 1, tmpdata, 1, 0);
  }
}
void rr() {
  for (uint8_t line = 0; line < 8; line++) {
    uint8_t tmpdata[1] = {right_door[line]};
    fisWriter.GraphicOut(41, line + 19 + 8, 1, tmpdata, 1, 0);
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

void redrawFrameBuffer() {
  //fisWriter.GraphicFromArray(0,0,64,88,frameBuffer,1);
  for (uint8_t line = 0; line < 88; line = line + 4) {
    fisWriter.initScreen(0x82, 0, 0, 64, 88);
    fisWriter.GraphicOut(0, line, 32, frameBuffer, 1, line * 8);
  }
}

void setup() {
  fisWriter.begin();
  fisWriter.initScreen(0x82, 0, 0, 1, 1);
  fisWriter.initScreen(0x82, 0, 0, 64, 88);
}


void loop() {  
  fisWriter.initScreen(0x82, 0, 0, 64, 88);
  fisWriter.GraphicFromArray(0, 0, 64, 88, b5f, 1);
  delay(3000);
  fisWriter.initScreen(0x82, 0, 0, 64, 88);
  fisWriter.GraphicFromArray(0, 0, 64, 65, Q, 1);
  fisWriter.GraphicFromArray(0, 70, 64, 16, QBSW, 1);
  delay(3000);
  fisWriter.initScreen(0x82, 0, 0, 1, 1);
  fisWriter.sendMsg("12345678  TEST  ");
  delay(100);
  fisWriter.initScreen(0x82, 0, 0x1B, 64, 0x30);
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
  //while(true){ //and display will did not change ever
  //  fisWriter.sendKeepAliveMsg();
  //  delay(1000);
  //}
}

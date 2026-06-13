/*
 * ENA pulse debug sketch
 *
 * Cycles through every display command in isolation so that ENA pulses
 * captured by DEBUG_ENA can be matched to the exact operation that caused them.
 *
 * Enable DEBUG_ENA in VAGFISWriter.cpp before flashing.
 *
 * Each step prints a labelled separator to Serial before sending, so the
 * Serial Monitor output looks like:
 *
 *   === initFullScreen(0x82) ===
 *   TX[9]: 53 06 82 00 00 40 58 00 CC [CRC]
 *   ENA pulse (char): latency=1980us width=18024us -> retry
 *   ...
 */

#include "VAGFISWriter.h"

#define FIS_CLK  SCK
#define FIS_DATA MOSI
#define FIS_ENA  49

VAGFISWriter fis(FIS_CLK, FIS_DATA, FIS_ENA);

// Simple 8-byte test bitmap (single horizontal line)
static const uint8_t testBitmap[8] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };

static void label(const __FlashStringHelper *s) {
  Serial.println();
  Serial.print(F("=== "));
  Serial.print(s);
  Serial.println(F(" ==="));
}

static void pause(uint16_t ms) {
  delay(ms);
}

void setup() {
  fis.begin(); // Serial.begin called inside when DEBUG_ENA is set
  pause(500);
  Serial.println(F("\n\n--- ENA pulse debug start ---"));
}

void loop() {

  // --- screen init variants ---
  label(F("initFullScreen(0x80) no-clear overlay"));
  fis.initScreen(0, 0, 64, 88, 0x80);  // draws over cluster's own UI, no clear
  pause(200);

  label(F("initFullScreen(0x81) no-clear variant"));
  fis.initScreen(0, 0, 64, 88, 0x81);
  pause(200);

  label(F("initFullScreen(0x82) clear positive"));
  fis.initScreen(0, 0, 64, 88, 0x82);
  pause(200);

  label(F("initFullScreen(0x83) clear negative/inverted"));
  fis.initScreen(0, 0, 64, 88, 0x83);
  pause(200);

  label(F("initMiddleScreen(0x82)"));
  fis.initMiddleScreen(0x82);
  pause(200);

  // --- fill and clear rectangle modes ---
  label(F("initScreen(0x03) filled rectangle"));
  fis.initScreen(0, 0, 64, 88, 0x82);  // clear screen first
  fis.initScreen(14, 26, 37, 50, 0x03); // fill outer rectangle solid
  pause(200);

  label(F("initScreen(0x02) clear inner -> rectangle outline"));
  fis.initScreen(15, 27, 35, 48, 0x02); // punch hole inside = border outline
  pause(500);

  label(F("initScreen(0x02) clear region only"));
  fis.initScreen(0, 0, 64, 88, 0x82);
  fis.initScreen(10, 10, 54, 78, 0x03); // fill
  fis.initScreen(20, 20, 44, 68, 0x02); // clear inside
  pause(500);

  // --- graphic output ---
  label(F("GraphicOut 8 bytes mode=0x01"));
  fis.initFullScreen(0x82);
  fis.GraphicOut(0, 0, 8, (const uint8_t*)testBitmap, 0x01);
  pause(200);

  label(F("GraphicOut 8 bytes mode=0x02"));
  fis.GraphicOut(0, 8, 8, (const uint8_t*)testBitmap, 0x02);
  pause(200);

  label(F("GraphicOut 8 bytes mode=0x03"));
  fis.GraphicOut(0, 16, 8, (const uint8_t*)testBitmap, 0x03);
  pause(200);

  // --- text display ---
  label(F("sendMsg spaces (radioDisplayBlank equivalent)"));
  fis.radioDisplayBlank();
  pause(200);

  label(F("sendMsg zeros (radioDisplayOff equivalent)"));
  fis.radioDisplayOff();
  pause(200);

  label(F("sendString two lines"));
  fis.sendString("TEST", "PULSE");
  pause(200);

  // --- reset (triggers ~18ms exit-ACK pulse from cluster; mode byte is ignored) ---
  label(F("reset() -> initScreen(0,0,1,1)"));
  fis.reset();
  pause(500);

  // --- keepalive ---
  label(F("sendKeepAliveMsg"));
  fis.sendKeepAliveMsg();
  pause(200);

  Serial.println(F("\n--- loop end ---"));
  pause(1000);
}

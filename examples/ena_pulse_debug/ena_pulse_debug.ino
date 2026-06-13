/*
 * ENA pulse debug sketch
 *
 * Cycles through every display command in isolation so that ENA pulses
 * captured by DEBUG_ENA can be matched to the exact operation that caused them.
 *
 * Enable DEBUG_ENA in VAGFISWriter.cpp before flashing.
 * Optionally enable DEBUG_CORRUPT_CRC to send wrong CRC on every packet.
 *
 * ENA pulse protocol (confirmed by DEBUG_ENA + DEBUG_CORRUPT_CRC tracing):
 *   - No pulse:    good CRC, packet accepted normally
 *   - ~6ms pulse:  CRC NAK — cluster rejected the packet, master must retry
 *   - ~18ms pulse: exit ACK — cluster exited graphics mode (packet had good CRC)
 *
 * 18ms exit-ACK rules:
 *   - First initScreen from idle: no pulse (enters graphics silently)
 *   - Any initScreen while already in graphics mode: 18ms pulse (exits old region)
 *   - reset() = initScreen(0,0,1,1): always 18ms pulse
 *
 * Other observations:
 *   - Unknown command bytes (0x99, 0x00, 0xFF): no ENA response at all
 *   - Malformed coords / short length: cluster accepts without complaint (no bounds check)
 *   - First packet of session may fail if cluster ENA is briefly HIGH at boot
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

  // -----------------------------------------------------------------------
  // CORRUPT DATA TESTS
  // Note: sendRawData always recomputes CRC from the data bytes - there is
  // no way to inject a bad CRC through it. A raw-byte sender would be needed.
  // -----------------------------------------------------------------------

  label(F("CORRUPT: unknown command 0x99"));
  fis.initFullScreen(0x82);
  { uint8_t pkt[] = { 0x99, 0x06, 0x82, 0x00, 0x00, 0x40, 0x58 };
    fis.sendRawData(pkt); }
  pause(500);

  label(F("CORRUPT: X2 < X1 (inverted coords)"));
  fis.initFullScreen(0x82);
  { uint8_t pkt[] = { 0x53, 0x06, 0x82, 0x40, 0x00, 0x00, 0x58 }; // X1=64 X2=0
    fis.sendRawData(pkt); }
  pause(500);

  label(F("CORRUPT: X2 > 64 (out of range)"));
  fis.initFullScreen(0x82);
  { uint8_t pkt[] = { 0x53, 0x06, 0x82, 0x00, 0x00, 0x7F, 0x58 }; // X2=127
    fis.sendRawData(pkt); }
  pause(500);

  label(F("CORRUPT: length field too small (3 instead of 6)"));
  fis.initFullScreen(0x82);
  { uint8_t pkt[] = { 0x53, 0x03, 0x82, 0x00 };
    fis.sendRawData(pkt); }
  pause(500);

  label(F("CORRUPT: all zeros"));
  fis.initFullScreen(0x82);
  { uint8_t pkt[] = { 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 };
    fis.sendRawData(pkt); }
  pause(500);

  label(F("CORRUPT: all 0xFF"));
  fis.initFullScreen(0x82);
  { uint8_t pkt[] = { 0xFF, 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    fis.sendRawData(pkt); }
  pause(500);

  fis.reset();
  pause(500);

  // -----------------------------------------------------------------------
  // COORDINATE BOUNDARY TESTS
  // -----------------------------------------------------------------------

  // --- A: init full screen, GraphicOut beyond hardware limits ---
  label(F("BOUNDS A: init full screen (0,0,64,88)"));
  fis.initFullScreen(0x82);
  pause(200);

  label(F("BOUNDS A: GraphicOut at X=65 (beyond screen width)"));
  fis.GraphicOut(65, 0, 8, testBitmap, 0x01);
  pause(300);

  label(F("BOUNDS A: GraphicOut at Y=89 (beyond screen height)"));
  fis.GraphicOut(0, 89, 8, testBitmap, 0x01);
  pause(300);

  label(F("BOUNDS A: GraphicOut at X=255 Y=255 (far out of range)"));
  fis.GraphicOut(255, 255, 8, testBitmap, 0x01);
  pause(300);

  label(F("BOUNDS A: GraphicOut starting inside but data extends past X=64"));
  fis.GraphicOut(60, 0, 8, testBitmap, 0x01); // 8 bytes = 64px wide, overflows right edge
  pause(300);

  label(F("BOUNDS A: GraphicOut starting inside but data extends past Y=88"));
  fis.GraphicOut(0, 85, 8, testBitmap, 0x01); // starts near bottom, extends below
  pause(300);

  fis.reset();
  pause(500);

  // --- B: init small square, GraphicOut outside that square ---
  label(F("BOUNDS B: init square region (10,10)-(40,40)"));
  fis.initFullScreen(0x82);           // clear whole screen first
  fis.initScreen(10, 10, 40, 40, 0x82); // init only this square
  pause(200);

  label(F("BOUNDS B: GraphicOut inside square (should work)"));
  fis.GraphicOut(10, 10, 4, testBitmap, 0x01);
  pause(300);

  label(F("BOUNDS B: GraphicOut at top-left (0,0) outside square"));
  fis.GraphicOut(0, 0, 4, testBitmap, 0x01);
  pause(300);

  label(F("BOUNDS B: GraphicOut at (50,50) outside square"));
  fis.GraphicOut(50, 50, 4, testBitmap, 0x01);
  pause(300);

  label(F("BOUNDS B: GraphicOut starting inside square extending outside"));
  fis.GraphicOut(10, 35, 8, testBitmap, 0x01); // Y=35 inside, 8 rows extends to Y=43 outside
  pause(300);

  fis.reset();
  pause(500);

  Serial.println(F("\n--- loop end ---"));
  pause(1000);
}

/*
 * initScreen two-step combination survey
 *
 * Tests all 64 combinations of base mode × sub-region mode:
 *   base modes:  0x80-0x87  (enter graphics mode)
 *   sub modes:   0x00-0x07  (sub-region operation applied after)
 *
 * Sequence per combination:
 *   1. reset()                              -- known clean state
 *   2. initScreen(0,0,64,88, base_mode)     -- enter graphics (full screen)
 *   3. initScreen(16,22,48,66, sub_mode)    -- sub-region op (centred rect)
 *   4. GraphicOut solid stripes full screen -- reveal what is writable
 *   5. pause for observation
 *
 * What to look for:
 *   - Does the base_mode clear (white), fill (red), or overlay (cluster native)?
 *   - Does the sub_mode immediately alter the centred rect (clear/fill/nothing)?
 *   - Do the GraphicOut stripes appear everywhere or only inside the centred rect?
 *
 * MODE BYTE IS BIT-BASED (confirmed by 8x8 combination survey):
 *
 *   bit 7 (0x80): 1 = session init (enter graphics mode for this rect)
 *                 0 = in-session sub-region op (must already be in graphics mode)
 *   bit 1 (0x02): 1 = perform fill or clear on the defined rect immediately
 *                 0 = no immediate pixel operation
 *   bit 0 (0x01): 0 = clear (pixels OFF / white)
 *                 1 = fill  (pixels ON  / red)
 *                 (only meaningful when bit 1 = 1)
 *   bits 2-6: no observed effect — cluster ignores them
 *
 * Only 6 distinct behaviours exist; all other values are aliases:
 *   0x80 = enter graphics, no fill/clear, normal polarity
 *   0x82 = enter graphics + clear                          [RECOMMENDED]
 *   0x83 = enter graphics + fill red (framebuffer 0xFF; GraphicOut XOR 0xFF = 0x00 = black)
 *   0x00 = define sub-region, no immediate pixel op
 *   0x02 = define sub-region + clear it                   [real-world confirmed]
 *   0x03 = define sub-region + fill it red                [real-world confirmed]
 *
 * Aliases: 0x01=0x00, 0x04=0x00, 0x05=0x00, 0x06=0x02, 0x07=0x03
 *          0x81=0x80, 0x84=0x80, 0x85=0x80, 0x86=0x82, 0x87=0x83
 *
 * Enable DEBUG_ENA in VAGFISWriter.cpp before flashing.
 *
 * Note: kFill MUST be PROGMEM — the const uint8_t* GraphicOut overload uses
 * memcpy_P. Without PROGMEM, random flash bytes are sent instead of 0xFF.
 *
 * ENA pulse legend:
 *   no pulse    packet accepted silently
 *   ~6ms pulse  CRC NAK (library retries automatically)
 *   ~18ms pulse exit ACK (cluster exited graphics mode)
 */

#include "VAGFISWriter.h"

#define FIS_CLK  SCK
#define FIS_DATA MOSI
#define FIS_ENA  49

VAGFISWriter fis(FIS_CLK, FIS_DATA, FIS_ENA);

// Solid 64px-wide row. MUST be PROGMEM: const uint8_t* GraphicOut uses memcpy_P.
static const uint8_t kFill[8] PROGMEM = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// Sub-region rectangle: centred on 64x88 screen, clearly inside but not full-screen.
#define SUB_X1  16
#define SUB_Y1  22
#define SUB_X2  48
#define SUB_Y2  66

// -----------------------------------------------------------------------
// Read one line from Serial, echoing each character as it arrives.
// -----------------------------------------------------------------------
static String serialReadLine() {
  while (Serial.available()) Serial.read();
  String s;
  for (;;) {
    if (!Serial.available()) continue;
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') { Serial.println(); break; }
    Serial.print(c);
    s += c;
  }
  return s;
}

static void printHex(uint8_t v) {
  if (v < 0x10) Serial.print('0');
  Serial.print(v, HEX);
}

// -----------------------------------------------------------------------
// Run one base+sub combination. Returns false if user wants to retry.
// -----------------------------------------------------------------------
static bool testCombo(uint8_t baseMode, uint8_t subMode) {
  Serial.println();
  Serial.print(F("=== base=0x")); printHex(baseMode);
  Serial.print(F(" sub=0x"));    printHex(subMode);
  Serial.println(F(" ==="));

  // Step 1: reset
  Serial.println(F("  [1] reset()"));
  fis.reset();
  delay(300);

  // Step 2: enter graphics mode (full screen)
  Serial.print(F("  [2] initScreen(0,0,64,88, 0x")); printHex(baseMode); Serial.println(')');
  fis.initScreen(0, 0, 64, 88, baseMode);
  delay(100);

  // Step 3: sub-region op on centred rect
  Serial.print(F("  [3] initScreen("));
  Serial.print(SUB_X1); Serial.print(',');
  Serial.print(SUB_Y1); Serial.print(',');
  Serial.print(SUB_X2); Serial.print(',');
  Serial.print(SUB_Y2);
  Serial.print(F(", 0x")); printHex(subMode); Serial.println(')');
  fis.initScreen(SUB_X1, SUB_Y1, SUB_X2, SUB_Y2, subMode);
  delay(100);

  // Step 4: draw solid stripes across the full screen
  Serial.println(F("  [4] GraphicOut 0xFF stripes full screen"));
  for (uint8_t y = 0; y < 88; y += 8) {
    fis.GraphicOut(0, y, 8, kFill, 0x01);
  }

  // Step 5: observe
  Serial.print(F(">>> base=0x")); printHex(baseMode);
  Serial.print(F(" sub=0x"));     printHex(subMode);
  Serial.print(F(" | what do you see? (ENTER=next, r=retry): "));

  String obs = serialReadLine();

  if (obs.equalsIgnoreCase("r")) {
    Serial.println(F("  [retrying]"));
    return false;
  }

  Serial.print(F("  OBS[0x")); printHex(baseMode);
  Serial.print(F("+0x"));      printHex(subMode);
  Serial.print(F("]: "));
  Serial.println(obs.length() > 0 ? obs : "(no note)");

  return true;
}

// -----------------------------------------------------------------------
// Mode tables
// -----------------------------------------------------------------------
static const uint8_t kBaseModes[] = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 };
static const uint8_t kSubModes[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

void setup() {
  fis.begin();
  delay(500);

  Serial.println(F("\n\n=== initScreen combination survey ==="));
  Serial.println(F("64 combinations: 8 base modes x 8 sub modes"));
  Serial.println(F("Sequence per test: reset -> initScreen(full,base) -> initScreen(rect,sub) -> stripes"));
  Serial.println(F("Sub-region rect: (16,22)-(48,66) centred on 64x88 screen"));
  Serial.println(F("ENTER = next  |  r = retry  |  type observation before ENTER to log it"));
  Serial.println();
  Serial.println(F("Press ENTER to begin."));
  serialReadLine();
}

void loop() {
  for (uint8_t b = 0; b < sizeof(kBaseModes); b++) {
    Serial.println();
    Serial.print(F("--- base mode 0x")); printHex(kBaseModes[b]);
    Serial.println(F(" ---"));

    for (uint8_t s = 0; s < sizeof(kSubModes); s++) {
      while (!testCombo(kBaseModes[b], kSubModes[s]));
    }
  }

  fis.reset();
  Serial.println(F("\n=== survey complete. Press ENTER to run again. ==="));
  serialReadLine();
}

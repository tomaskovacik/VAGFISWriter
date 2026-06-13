/*
 * initScreen mode survey
 *
 * Systematically tests every initScreen mode byte and pauses after each one
 * so you can observe the cluster and type your notes into the Serial console.
 * Your input is echoed back so the full log captures both the protocol output
 * (from DEBUG_ENA) and your visual observations in one place.
 *
 * Enable DEBUG_ENA in VAGFISWriter.cpp before flashing.
 *
 * Workflow per mode:
 *   1. reset() — exit graphics mode and blank display (known baseline)
 *   2. apply the test mode with initScreen(0, 0, 64, 88, mode)
 *   3. draw solid 0xFF horizontal stripes across the whole screen with GraphicOut
 *   4. pause — type what you see + ENTER to advance, 'r' + ENTER to retry
 *
 * Working hypothesis (needs further verification):
 *
 *   STEP 1 — enter graphics mode with an 0x8x mode:
 *     0x80  Base overlay init. Overlays cluster display without clearing.
 *           Confirmed to write over cluster text rows — no write protection.
 *           (Earlier "rejected rows" were communication errors, not real protection.)
 *     0x82  Like 0x80 but clears region first (recommended for clean graphics).
 *     0x83  Like 0x80 but fills region first (inverted polarity).
 *     0x81, 0x84-0x87  Variants; exact differences not yet confirmed.
 *
 *   STEP 2 — optionally init sub-rectangles using 0x0x modes (after 0x80):
 *     0x01  Init sub-region (exact behaviour under investigation).
 *     0x02  Clear (pixels OFF) the specified rectangle within the session.
 *     0x03  Fill  (pixels ON)  the specified rectangle within the session.
 *     0x00, 0x04-0x07  No confirmed effect.
 *
 * ENA pulse legend (from library):
 *   no pulse    packet accepted silently
 *   ~6ms pulse  CRC NAK (library retries automatically)
 *   ~18ms pulse exit ACK (cluster exited graphics mode)
 *
 * Note: kFill MUST be PROGMEM — the const uint8_t* GraphicOut overload uses
 * memcpy_P. Without PROGMEM, random flash bytes are sent instead of 0xFF.
 */

#include "VAGFISWriter.h"

#define FIS_CLK  SCK
#define FIS_DATA MOSI
#define FIS_ENA  49

VAGFISWriter fis(FIS_CLK, FIS_DATA, FIS_ENA);

// All-on pattern: 8 bytes of 0xFF = solid 64px-wide row.
// MUST be PROGMEM: the const uint8_t* GraphicOut overload uses memcpy_P.
static const uint8_t kFill[8] PROGMEM = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// -----------------------------------------------------------------------
// Read one line from Serial, echoing each character as it arrives.
// Blocks until '\n' is received. Returns the typed string (without '\n').
// -----------------------------------------------------------------------
static String serialReadLine() {
  while (Serial.available()) Serial.read(); // discard stale bytes
  String s;
  for (;;) {
    if (!Serial.available()) continue;
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') { Serial.println(); break; }
    Serial.print(c); // echo so terminal shows what user typed
    s += c;
  }
  return s;
}

// -----------------------------------------------------------------------
// Run one mode test. Returns false if user wants to retry (typed 'r').
// -----------------------------------------------------------------------
static bool testMode(uint8_t mode) {
  Serial.println();
  Serial.print(F("--- mode 0x"));
  if (mode < 0x10) Serial.print('0');
  Serial.println(mode, HEX);

  // Step 1: reset to known state (exits graphics mode, blanks display)
  Serial.println(F("  [reset: exit graphics mode + blank display]"));
  fis.reset();
  delay(300);

  // Step 2: apply test mode
  Serial.print(F("  [test:  initScreen(0,0,64,88, 0x"));
  if (mode < 0x10) Serial.print('0');
  Serial.print(mode, HEX);
  Serial.println(F(")]"));
  fis.initScreen(0, 0, 64, 88, mode);
  delay(100);

  // Step 3: draw solid stripes — if mode entered graphics mode, these appear
  Serial.println(F("  [draw:  GraphicOut stripes across full screen]"));
  for (uint8_t y = 0; y < 88; y += 8) {
    fis.GraphicOut(0, y, 8, kFill, 0x01);
  }

  // Step 4: wait for observation
  Serial.print(F(">>> 0x"));
  if (mode < 0x10) Serial.print('0');
  Serial.print(mode, HEX);
  Serial.print(F(": what do you see? (ENTER=next, r=retry): "));

  String obs = serialReadLine();

  if (obs.equalsIgnoreCase("r")) {
    Serial.println(F("  [retrying...]"));
    return false;
  }

  // log the observation alongside the mode so the full serial log is self-contained
  Serial.print(F("  OBS[0x"));
  if (mode < 0x10) Serial.print('0');
  Serial.print(mode, HEX);
  Serial.print(F("]: "));
  if (obs.length() > 0) {
    Serial.println(obs);
  } else {
    Serial.println(F("(no note)"));
  }

  return true;
}

// -----------------------------------------------------------------------
// Modes to survey — add or remove as needed
// -----------------------------------------------------------------------
static const uint8_t kModes[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
};

void setup() {
  fis.begin(); // Serial.begin(115200) called inside when DEBUG_ENA is set
  delay(500);

  Serial.println(F("\n\n=== initScreen mode survey ==="));
  Serial.println(F("Workflow: clear(0x82) -> initScreen(mode) -> draw stripes -> observe"));
  Serial.println(F("ENTER = advance to next mode"));
  Serial.println(F("'r'   = retry same mode"));
  Serial.println(F("type any text before ENTER to log your observation"));
  Serial.println();
  Serial.print(F("Testing "));
  Serial.print((uint8_t)sizeof(kModes));
  Serial.println(F(" modes. Press ENTER to begin."));
  serialReadLine();
}

void loop() {
  for (uint8_t i = 0; i < sizeof(kModes); i++) {
    while (!testMode(kModes[i]));
  }

  fis.reset();
  Serial.println(F("\n=== survey complete — press ENTER to run again ==="));
  serialReadLine();
}

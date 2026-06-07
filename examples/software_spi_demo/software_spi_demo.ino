/*
 * VAGFISWriter Software SPI Demo
 * 
 * This example demonstrates using software SPI with non-hardware SPI pins.
 * The library automatically detects if you're using hardware SPI pins or not.
 * 
 * Hardware SPI (faster, recommended):
 * - Arduino Uno/Nano: CLK=pin 13 (SCK), DATA=pin 11 (MOSI)
 * - STM32: CLK=PA5/PB3, DATA=PA7/PB5 (depends on board)
 * - ESP32: CLK=pin 18, DATA=pin 23 (default VSPI)
 * 
 * Software SPI (works with any digital pins, slower):
 * - Any digital pins can be used as demonstrated below
 * - Useful when hardware SPI pins are already in use
 * 
 * ENA pin requirements:
 * - Can be any digital pin with interrupt capability (for mode 0)
 * - Or any digital pin if using force mode (4th parameter = 1)
 */

#include "VAGFISWriter.h"

// Using non-hardware SPI pins for demonstration
// This will automatically use software SPI
#define FIS_CLK 10   // Any digital pin
#define FIS_DATA 9   // Any digital pin
#define FIS_ENA 2    // Digital pin with interrupt capability

VAGFISWriter fisWriter(FIS_CLK, FIS_DATA, FIS_ENA);

void setup() {
  Serial.begin(115200);
  fisWriter.begin();
  
  // Check which SPI mode is being used
  if (fisWriter.isUsingHardwareSPI()) {
    Serial.println("Using Hardware SPI (fast)");
  } else {
    Serial.println("Using Software SPI (slower, but works with any pins)");
  }
  
  // Initialize display
  fisWriter.initMiddleScreen();
  delay(100);
}

void loop() {
  // Display some text
  fisWriter.sendString("SOFTWARE", "SPI DEMO");
  delay(2000);
  
  fisWriter.sendString("PIN CLK", String(FIS_CLK));
  delay(2000);
  
  fisWriter.sendString("PIN DATA", String(FIS_DATA));
  delay(2000);
}

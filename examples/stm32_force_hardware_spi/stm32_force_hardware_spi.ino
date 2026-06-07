/*
 * VAGFISWriter - Force Hardware SPI Example
 * 
 * This example shows how to manually override SPI mode detection.
 * Useful for:
 * - STM32 boards where auto-detection might not work
 * - Using alternate SPI pins that are hardware-capable but not detected
 * - ESP32 with remapped SPI pins
 * - Any board where you know better than auto-detection
 * 
 * IMPORTANT: The #define MUST come BEFORE including the library!
 */

// Force hardware SPI mode (even if auto-detection would choose software)
#define VAGFISWRITER_FORCE_HARDWARE_SPI

// OR force software SPI mode:
// #define VAGFISWRITER_FORCE_SOFTWARE_SPI

#include "VAGFISWriter.h"

// Example: STM32 Blue Pill with SPI1
// These ARE hardware SPI pins, but might not be auto-detected on all cores
#define FIS_CLK PA5    // SPI1 SCK
#define FIS_DATA PA7   // SPI1 MOSI
#define FIS_ENA PA15   // Any pin with interrupt capability

// Example: STM32 with SPI2
//#define FIS_CLK PB13   // SPI2 SCK
//#define FIS_DATA PB15  // SPI2 MOSI
//#define FIS_ENA PA15

VAGFISWriter fisWriter(FIS_CLK, FIS_DATA, FIS_ENA);

void setup() {
  Serial.begin(115200);
  fisWriter.begin();
  
  // Verify which mode is active
  if (fisWriter.isUsingHardwareSPI()) {
    Serial.println("Hardware SPI FORCED - using SPI peripheral");
  } else {
    Serial.println("Software SPI active - bit-banging");
  }
  
  // Initialize display
  fisWriter.initMiddleScreen();
  delay(100);
}

void loop() {
  fisWriter.sendString("FORCED", "HW SPI");
  delay(2000);
  
  fisWriter.sendString("STM32", "EXAMPLE");
  delay(2000);
}

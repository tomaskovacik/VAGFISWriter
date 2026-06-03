#include "VAGFISWriter.h"

/*
 * Hardware SPI pins must be used for CLK and DATA:
 * - Arduino Uno/Nano: CLK=pin 13 (SCK), DATA=pin 11 (MOSI)
 * - ENA can be any digital pin with interrupt capability
 * 
 * If ENA pin does not have HW interrupt, force mode must be set in initialization (4th parameter = 1)
 * If no real radio/cluster is used, PULL_DOWN must be applied on ENABLE line!
 */
#define FIS_CLK 13   // SPI SCK (hardware SPI required)
#define FIS_DATA 11  // SPI MOSI (hardware SPI required)
#define FIS_ENA 2    // Digital pin with interrupt capability

//HW interrupt on FIS_ENA pin
VAGFISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA,0);
//no HW interrupt on FIS_ENA pin
//VAGFISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA,1);

void setup() {
  fisWriter.begin();
  fisWriter.reset();
}
void loop() {

fisWriter.sendRadioMsg("TEST12345678TEST");

while(1) {
    //no HW interrupt on FIS_ENA pin
    //we must time outputing packets to cluster
    //delay(1500);// should be send each second and should be writen using uint32_t delay=millis()+1500;while (millis()<delay);
    fisWriter.sendRadioData();
}

}

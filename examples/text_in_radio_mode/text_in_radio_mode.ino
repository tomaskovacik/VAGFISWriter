#include "VAGFISWriter.h"

#define FIS_ENA 2  //if this pin did not have HW interrupt, force mode must by set in initialization of VAGFISWRITER (4th parameter = 1) 
#define FIS_CLK 6
#define FIS_DATA 7

//HW interrupt on FIS_ENA pin
VAGFISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA,0);
//no HW interrupt on FIS_ENA pin
//VAGFISWriter fisWriter( FIS_CLK, FIS_DATA, FIS_ENA,0);

void setup() {
  fisWriter.begin();
  fisWriter.reset();
}
long time = 0;
void loop() {

  fisWriter.sendRadioMsg("TEST12345678TEST");

while(1) {
    //no HW interrupt on FIS_ENA pin
    //we must time outputing packets to cluster
    //delay(1500);// should be send each second
    fisWriter.sendRadioData();
}

}

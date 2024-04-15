#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Logger.h>
#include "Locations.h"
#include "Display.hpp"

class CasioClock {

/*
 * public methods
 */
public:
boolean forceRefresh = false;
int whichTime1224 = 12;

// constructor
CasioClock();

// time
void ntpOff();
void ntpOn();
void localize();
boolean update();
boolean changeMapLocation(char* code = NULL);
boolean changePrimeLocation(char* code = NULL);
boolean isTimeSet();
unsigned long getEpochTime();
int getSeconds();

// display
void init();
void idle();
int refresh();
boolean setDisplay(int turnOn = -1);

// control
int setMode(int argc = 0, char* argv[] = NULL);
int nextSetting(int argc = 0, char* argv[] = NULL);
int changeTimeFormat(int argc = 0, char* argv[] = NULL);

// info
String lvglVersion();

private:
Locations clockLocations;
int mapLocationIx = 1;
int primeLocationIx = 0;
Display* display;
int currentMode = 0;

};
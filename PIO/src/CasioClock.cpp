/*
 * CasioClock implementation
 */
#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Logger.h>
#include "CasioClock.hpp"
#include "Display.hpp"
#include "Locations.h"

#define NTP_SERVER1           "pool.ntp.org"
#define NTP_SERVER2           "time.nist.gov"
#define GMT_OFFSET_SEC        (long)0
#define UPDATE_INTERVAL       (unsigned long)60000

extern char* fmt(const char* fmt, ...);

/*
 * public
 */
boolean forceRefresh;           // triggered by localizer task in Locations
WiFiUDP ntpUDP;
// NB: timeClient should be private, but cannot (yet) make this a private member
NTPClient timeClient = NTPClient(ntpUDP, NTP_SERVER1, GMT_OFFSET_SEC, UPDATE_INTERVAL); 

/*
 * private
 */
Locations clockLocations;
int mapLocationIx = 1;
int primeLocationIx = 0;
Display* display;
int currentMode = 0;  // 0: Lock, 1: changeMapTime, 1: changePrimeTime, 2: changeBG
int timeFormat = 12;

CasioClock::CasioClock() {
    currentMode = 0;
    timeFormat = 12;
    display = new Display(clockLocations.getLocation(mapLocationIx), clockLocations.getLocation(primeLocationIx));
}

void CasioClock::ntpOff() {
    timeClient.end();
}

void CasioClock::ntpOn() {
    timeClient.begin();
}

void CasioClock::localize() {
    clockLocations.localize();
}

void CasioClock::init() {
    display->init();
}

void CasioClock::idle() {
    display->idle();
}

int CasioClock::refresh() {
    if(forceRefresh) {
        display->forceRefresh = true;
        forceRefresh = false;
    }
    return display->refresh();
}

boolean CasioClock::setDisplay(int turnOn) {
    return display->setDisplay(turnOn);
}

void CasioClock::update() {
    timeClient.update();
}

boolean CasioClock::changeMapLocation(char *name) {
    if(name != NULL) {
        int newLoc = clockLocations.ixByName(name);
        if(newLoc == -1) {
            Logger::warning(fmt("invalid location name '%s'", name));
            return false;
        } else {
            mapLocationIx = newLoc;
        }
    } else {
        mapLocationIx = clockLocations.nextLocation(mapLocationIx);
    }
    Logger::notice(fmt("New location for Map time - index: %d", mapLocationIx));
    display->setMapLocation(clockLocations.getLocation(mapLocationIx));
    display->refreshMapTime();
    return true;
}

boolean CasioClock::changePrimeLocation(char *name) {
    if(name != NULL) {
        int newLoc = clockLocations.ixByName(name);
        if(newLoc == -1) {
            Logger::warning(fmt("invalid location name '%s'", name));
            return false;
        } else {
            primeLocationIx = newLoc;
        }
    } else {
        primeLocationIx = clockLocations.nextLocation(primeLocationIx);
    }
    Logger::notice(fmt("New location for Prime time - index: %d", primeLocationIx));
    display->setPrimeLocation(clockLocations.getLocation(primeLocationIx));
    display->refreshPrimeTime();
    return true;
}

boolean CasioClock::isTimeSet() {
    return timeClient.isTimeSet();
}

unsigned long CasioClock::getEpochTime() {
    return timeClient.getEpochTime();
}

int CasioClock::getSeconds() {
    return timeClient.getSeconds();
}

int CasioClock::setMode(int argc, char* argv[]) {
    // Set the current action mode if a parmeter was provided
    if(argc == 2) {
        if(strcmp(argv[1], "lock") == 0) {
            currentMode = 0;
        } else if(strcmp(argv[1], "map") == 0) {
            currentMode = 1;
        } else if(strcmp(argv[1], "prime") == 0) {
            currentMode = 2;
        } else if(strcmp(argv[1], "format") == 0) {
            currentMode = 3;
        } else if(strcmp(argv[1], "color") == 0) {
            currentMode = 4;
        } else if(strcmp(argv[1], "next") == 0) {
            currentMode = ++currentMode % 5;
        } else {
            Logger::error(fmt("invalid mode'%s'", argv[1]));
        }
    } else {
    // Increment the current action state, and loop back to 0 after the last action
        currentMode = ++currentMode % 5;
    }
    display->showMode(currentMode);
    return 0;
}

int CasioClock::changeTimeFormat(int argc, char* argv[]) {
    int rval = 0;
    switch(argc) {
        // null arguments - button push - advance to next setting
        case 0:
            display->timeFormat = (display->timeFormat == 12 ? 24 : 12);
            display->forceRefresh = true;
            break;
        // no command arguments - just show the setting
        case 1:
            break;
        // 2 arguments - show or set the time format
        case 2:
            if(strcmp(argv[1], "show") == 0) {
                    // fall through to show
            } else {
                int which = atoi(argv[1]);
                switch(which) {
                    case 12:
                    case 24:
                        display->timeFormat = which;
                        display->forceRefresh = true;
                        break;
                    default:
                        Logger::error(fmt("%s: invalid time format %s", argv[0], argv[1]));
                        rval = 1;
                }
            }
            break;
        // extra arguments
        default:
            Logger::error(fmt("%s: too many arguments (%d)", argv[0], argc));
            rval = 2;
    }
    // show the format
    Logger::notice(fmt("time format %d", display->timeFormat));
    return rval;
}

// Execute the current action based on the current state
int CasioClock::nextSetting(int argc, char* argv[]) {
    switch(currentMode) {
        case 0:
            Logger::notice("Locked");
            break;
        case 1:
            changeMapLocation();
            break;
        case 2:
            changePrimeLocation();
            break;
        case 3:
            changeTimeFormat();
            break;
        case 4:
            display->changeBG();
            break;
    }
    return 0;
}

String CasioClock::lvglVersion() {
    return display->lvglVersion();
}

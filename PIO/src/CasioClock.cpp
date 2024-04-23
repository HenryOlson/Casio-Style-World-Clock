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
Preferences prefClock;

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
    prefClock.begin("clock");
    if(prefClock.isKey("format")) {
        int format = prefClock.getInt("format");
        setTimeFormat(format);
    }
    prefClock.end();
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

boolean CasioClock::update() {
    Logger::notice("checking network time");
    if(!timeClient.update()) {
        Logger::warning("Unable to update time");
        return false;
    }
    return true;
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

int CasioClock::getMode() {
    return currentMode;
}

void CasioClock::setMode(int newMode) {
    if(newMode == -1)
        // next mode
        currentMode = ++currentMode % 5;
    else
        currentMode = newMode;
    display->showMode(currentMode);
}

int CasioClock::setTimeFormat(int format) {
    switch(format) {
        case 0:
            display->timeFormat = (display->timeFormat == 12 ? 24 : 12);
            break;
        case 12:
        case 24:
            display->timeFormat = format;
            prefClock.begin("clock");
            prefClock.putInt("format", format);
            prefClock.end();
            break;
        default:
            Logger::error(fmt("setTimeFormat: invalid time format %d", format));
            return -1;
    }
    display->forceRefresh = true;
    return display->timeFormat;
}

int CasioClock::getTimeFormat() {
    return display->timeFormat;
}

// Execute the current action based on the current state
void CasioClock::nextSetting() {
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
            setTimeFormat();
            break;
        case 4:
            display->changeBG();
            break;
    }
}

String CasioClock::lvglVersion() {
    return display->lvglVersion();
}

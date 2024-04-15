/*
 * Locations class handles lat / lon and timezones for the CasioClock
 * includes "localize" method to configure a local zone based on IP-related information
 */
#include <Arduino.h>
#include <Logger.h>
#include "Locations.h"
#include "CasioClock.hpp"
#include "IpAPI.hpp"
#include "MercatorPlotter.hpp"
#include "zones.h"

#define LOCALIZE_PRIORITY 12

// NB: the three-letter codes are a mix of country codes, airport codes and city name abbreviations
//     they are intuitive, rather than uniform
Location locations[] = {
    {"GMT0", "GMT", 69, 44},                                  // Null Island -> Localtime
    {"GMT0", "GMT", 69, 44},                                  // Null Island / GMT
    {"PST8PDT,M3.2.0,M11.1.0", "SFO", 21, 28},                // America/Los_Angeles
    {"EST5EDT,M3.2.0,M11.1.0", "NYC", 40, 27},                // America/New_York
    {"<-03>3", "ARG", 48, 58},                                // America/Argentina/Buenos_Aires
    {"GMT0BST,M3.5.0/1,M10.5.0", "LHR", 69, 21},              // Europe/London
    {"CET-1CEST,M3.5.0,M10.5.0/3", "DEU", 78, 19},            // Europe/Berlin
    {"<+03>-3", "BHR", 88, 34},                               // Asia/Bahrain
    {"IST-5:30", "IND", 103, 35},                             // Asia/Kolkata
    {"JST-9", "TYO", 123, 29},                                // Asia/Tokyo
    {"AEST-10AEDT,M10.1.0,M4.1.0/3", "AUS", 127, 58},         // Australia/Sydney
};

Location* Locations::getLocation(int i) {
    return &(locations[i % (sizeof(locations) / sizeof(locations[0]))]);
}

int Locations::ixByName(char* name) {
    int nLoc = (sizeof(locations) / sizeof(locations[0]));
    for(int i=0; i<nLoc; i++) {
        if(strcmp(name, locations[i].locationCode) == 0) {
            return i;
        }
    }
    return -1;
}

int Locations::nextLocation(int current) {
    return (current + 1) % (sizeof(locations) / sizeof(locations[0]));
}

static TaskHandle_t  vUpdateDateTimeTaskHandler = NULL;
void localizeTask(void *ptr);
void Locations::localize() {
    Logger::verbose("checking for location update task");
    if (!vUpdateDateTimeTaskHandler) {
        Logger::verbose("updating location");
        xTaskCreate(localizeTask, "localize", 10 * 1024, &(locations[LOCAL_INDEX]), LOCALIZE_PRIORITY,  &vUpdateDateTimeTaskHandler);
    }
}

/*
 * localization support functions
 * would rather these were private class methods, but can't seem to make those inline
 * 
 */
// external references for localization
extern CasioClock theClock;
extern char* fmt(const char* fmt, ...);

// NB: routines called by FreeRTOS tasks must be inline
void inline setLocation(zones_t localZone, MapPoint mapPoint, Location* local) __attribute__ ((always_inline));
void setLocation(zones_t localZone, MapPoint mapPoint, Location* local) {

    char buf[60];
    const char* tz = localZone.zones.c_str();

    // determine offset hours
    char zoneCode[20];
    int offsetHrs;
    sscanf(tz, "%[^0-9+-]%d*s", &zoneCode, &offsetHrs);
    if(zoneCode[0] != '<')
        offsetHrs = -1 * offsetHrs;
    sprintf(buf, "%+d", offsetHrs);

    // configure the local time location
    local->locationCode = "LCL";
    local->tz = strdup(tz);
    local->locX = mapPoint.x;
    local->locY = mapPoint.y;

    // log configuration
    Logger::verbose(fmt("%s tz: %s", localZone.name.c_str(), tz));
    Logger::verbose("local time set");

}

// NB: routines called by FreeRTOS tasks must be inline
void inline handleIPData(IpAPI* ipData, Location* local) __attribute__ ((always_inline));
void inline handleIPData(IpAPI* ipData, Location* local) {
    const char* timeZone = ipData->get("timezone");
    double latitude = ipData->get("latitude");
    double longitude = ipData->get("longitude");
    Logger::notice(fmt("local time (LOC) - timeZone: %s, lat: %f, lon: %f", timeZone, latitude, longitude));
    MercatorPlotter plotter = MercatorPlotter(143,70,370);
    plotter.setTopLeft(-1,-2);
    plotter.setNullIslandOffset(10,0);
    MapPoint mapPoint = plotter.plot(latitude, longitude);
    Logger::verbose(fmt("map coordinates: %s", mapPoint.toString()));
    for (uint32_t i = 0; i < (sizeof(zones)/sizeof(zones_t)); i++) {
        if (zones[i].name == timeZone) {
            setLocation(zones[i], mapPoint, local);
            break;  // for
        }
    }
}

void localizeTask(void *ptr) {

    Logger::verbose("getting location");
    Location* local = (Location*) ptr;

    IpAPI ipData = IpAPI();

    int retries = 3;
    while (--retries >= 0) {

        if(ipData.isLoaded()) {
            handleIPData(&ipData, local);
            theClock.forceRefresh = true;
            Logger::notice("local timezone set");
            break;  // while
        } else {
          // try again
          delay(3000);
          Logger::warning("location data not loaded, retrying");
          ipData.fetch();
        }
    }
    if(retries < 0) {
          Logger::error("IP lookup failed, location not set");
    }

    // Run task just once
    // delete self
    vUpdateDateTimeTaskHandler = NULL;
    vTaskDelete(NULL);
    Logger::notice("location sync task complete");
}

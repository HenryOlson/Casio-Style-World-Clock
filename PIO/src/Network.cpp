/*
 * WiFi network initialization and event handling for the CasioClock
 */
#include <Arduino.h>
#include <Logger.h>
#include "Network.h"
#include "CasioClock.hpp"

#define WIFI_KEY "WiFi"

// WiFi configuration
char Network::_ssid[MAX_SSID];
char Network::_password[MAX_PW];
Preferences Network::preferences;

// required coupling
extern CasioClock theClock;
extern char* fmt(const char* fmt, ...);

void Network::WiFiEvent(WiFiEvent_t event) {

    // NTP retries
    int retries = 4;
    IPAddress ip;

    Logger::verbose(fmt("wifi event %d", event));

    switch (event) {

        case ARDUINO_EVENT_WIFI_READY:
            Logger::notice("WiFi interface ready");

            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Logger::notice("Completed scan for access points");
            break;

        case ARDUINO_EVENT_WIFI_STA_START:
            Logger::notice("WiFi client started");
            break;

        case ARDUINO_EVENT_WIFI_STA_STOP:
            Logger::notice("WiFi clients stopped");
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Logger::notice("Connected to access point");
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Logger::notice("Disconnected from WiFi access point");
            break;

        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Logger::notice("Authentication mode of access point has changed");
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Logger::verbose("begin WiFi IP event");
            ip = WiFi.localIP();
            Logger::notice(fmt("Obtained IP address: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]));

            // Save network credentials if changed
            preferences.begin(WIFI_KEY);
            Logger::verbose(fmt("checking ssid; stored: '%s' current: '%s'", preferences.getString("ssid").c_str(), _ssid));
            if(strcmp(preferences.getString("ssid").c_str(), _ssid) != 0 ||
                strcmp(preferences.getString("password").c_str(), _password) != 0) {
                    Logger::verbose(fmt("saving credentials for %s", _ssid));
                    preferences.putString("ssid", _ssid);
                    preferences.putString("password", _password);
                    preferences.end();
            } else {
                    Logger::verbose(fmt("wifi credentials unchanged for %s", _ssid));
            }

            // get network time
            Logger::verbose("getting network time");
            do {
                theClock.update();
                delay(200);
            } while(!theClock.isTimeSet() && retries-- > 0);
            if(theClock.isTimeSet()) {
                Logger::notice("got network time");
            } else {
                Logger::error("could not get network time");
            }

            // show network (GMT) time
            Logger::verbose("trigger display update for network (GMT) time");
            theClock.forceRefresh = true;
            theClock.refresh();

            // set local timezone
            theClock.localize();

            Logger::verbose("end WiFi IP event");
            break;

        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Logger::notice("Lost IP address and IP address is reset to 0");
            break;

        default:
            break;

    }
}

boolean Network::initWiFi(char* ssid, char* password) {

    // parameters
    if(ssid == NULL || password == NULL) {
        // get preferences
        Logger::verbose("loading saved WiFi credentials");
        preferences.begin(WIFI_KEY);
        String pSSID = preferences.getString("ssid");
        String pPASS = preferences.getString("password");
        if(pSSID.length() > 0 && pPASS.length() > 0) {
            strcpy(_ssid, pSSID.c_str());
            strcpy(_password, pPASS.c_str());
        } else {
            Logger::notice("need credentials, cannot initWiFi");
            return false;
        }
    } else {
        strcpy(_ssid, ssid);
        strcpy(_password, password);
    }

    // configure
    Logger::notice("initWiFi");
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    // Register WiFi callback
    WiFi.onEvent(WiFiEvent);

    // connect
    Logger::notice(fmt("wifi connecting to '%s'", _ssid));
    WiFi.begin(_ssid, _password);

    Logger::verbose("initWiFi complete");
    return true;

}

void Network::disconnect() {
    WiFi.disconnect();
}

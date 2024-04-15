/*
 * interface to the IP API service
 */
#include <Logger.h>
#include "IpAPI.hpp"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "rootCa.h"
#define GET_LOCATION_API      "https://ipapi.co/json"

/*
 * private
 */
WiFiClientSecure client;

IpAPI::IpAPI(boolean loadNow) {
    client.setCACert(rootCACertificate);
    if(loadNow) {
        this->fetch();
    }
}

boolean IpAPI::isLoaded() {
    return this->loaded;
}

JsonVariant IpAPI::get(const char* name) {
    return this->ipData[name];
}

bool IpAPI::fetch() {
    int rval = false;
    HTTPClient https;
    if (https.begin(client, GET_LOCATION_API)) {
        int httpCode = https.GET();
        if (httpCode > 0) {
            Logger::verbose(fmt("[HTTPS] GET... code: %d", httpCode));
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                this->httpBody = https.getString();
                DeserializationError error = deserializeJson(this->ipData, this->httpBody.c_str());
                if (error) {
                    Logger::error(fmt("deserializeJson() failed: %s", error.f_str()));
                    this->errorMessage = error.f_str();
                } else {
                    Logger::verbose("parsed API response");
                    rval = true;
                    this->loaded = true;
                }
            }
        } else {
            Logger::error(fmt("[HTTPS] GET... failed, code: %d, error: %s",
                        httpCode, https.errorToString(httpCode).c_str()));
            this->errorMessage = https.errorToString(httpCode).c_str();
        }
        https.end();
    }
    return rval;
}

char* IpAPI::fmt(const char* fmt, ...) {
    static char buf[200];
    va_list va;
    va_start (va, fmt);
    vsprintf (buf, fmt, va);
    va_end (va);
    return buf;
}


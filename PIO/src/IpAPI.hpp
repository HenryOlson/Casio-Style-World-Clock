#ifndef IPAPI_H
#define IPAPI_H
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

class IpAPI {
    JsonDocument ipData;
    boolean loaded = false;
    String errorMessage;
    String httpBody;
  public:
    IpAPI(boolean loadNow = true);
    boolean isLoaded();
    JsonVariant get(const char* name);
    bool fetch();
  private:
    char* fmt(const char* fmt, ...);
    WiFiClientSecure client;
};
#endif
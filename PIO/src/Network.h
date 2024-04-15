#ifndef NETWORK_H
#define NETWORK_H
#include <WiFi.h>
#include <Preferences.h>
#define MAX_SSID 256
#define MAX_PW 256

class Network {
    public:
    static boolean initWiFi(char* ssid = NULL, char* password = NULL);
    static void disconnect();
    private:
    static char _ssid[MAX_SSID];
    static char _password[MAX_PW];
    static Preferences preferences;
    static void WiFiEvent(WiFiEvent_t event);
};
#endif
#ifndef NETWORK_H
#define NETWORK_H
#include <WiFi.h>
#include <Preferences.h>
#include "ESPAsyncWebServer.h"
#include "CLI.h"
#define MAX_SSID 256
#define MAX_PW 256

class Network {
    public:
    static boolean initWiFi(char* ssid = NULL, char* password = NULL);
    static void disconnect();

    static boolean webCLIEnabled;
    static boolean initWebCLI(const char* hostName, const char* prompt);
    static boolean setWebAuth(char* id, char* password);
    static boolean enableWebCLI();
    static boolean disableWebCLI();

    private:
    static char _ssid[MAX_SSID];
    static char _password[MAX_PW];
    static Preferences prefWiFi;
    static Preferences prefWeb;
    static CLIClient* cliClient;
    static String webID;
    static String webPassword;

    static String cliHost;
    static String hostIP;
    static String cliPrompt;
    //AsyncWebServer server;
    static String processor(const String& var);

    static void WiFiEvent(WiFiEvent_t event);
};
#endif
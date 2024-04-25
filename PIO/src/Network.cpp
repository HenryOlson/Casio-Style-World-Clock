/*
 * WiFi network initialization and event handling for the CasioClock
 */
#include <Arduino.h>
#include <Logger.h>
#include <ESPmDNS.h>
#include "Network.h"
#include "CasioClock.hpp"
#include "SPIFFS.h"
#include "RWBufferStream.h"

#define WIFI_KEY "WiFi"
#define WEB_AUTH_KEY "WebAuth"

// required coupling
extern CasioClock theClock;
extern char* fmt(const char* fmt, ...);

// WiFi configuration
char Network::_ssid[MAX_SSID];
char Network::_password[MAX_PW];
Preferences Network::prefWiFi;
Preferences Network::prefWeb;

// web CLI
String Network::cliHost;
String Network::hostIP;
String Network::cliPrompt;
AsyncWebServer server(80);
RWBufferStream cliBuffer(200,1500);
String Network::webID = "admin";
String Network::webPassword = "p@55";

// page processor
String Network::processor(const String& var) {
  if(var == "HOST"){
    return cliHost;
  } else if(var == "HOST_IP"){
    return hostIP;
  } else if(var == "CLI_PROMPT"){
    return cliPrompt;
  }
  return String();
}

CLIClient* Network::cliClient;
boolean Network::initWebCLI(const char* hostName, const char* prompt) {

    cliHost = String(hostName);
    cliPrompt = String(prompt);

    Logger::notice("Initializing Web CLI");

    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        Logger::error("An Error has occurred while mounting SPIFFS");
        return false;
    }

    // get Web CLI credentials and status
    Logger::verbose("checking saved Web credentials");
    prefWeb.begin(WEB_AUTH_KEY);
    String _webID = prefWeb.getString("id");
    String _webPassword = prefWeb.getString("password");
    if(_webID.length() > 0 && _webPassword.length() > 0) {
        webID = _webID;
        webPassword = _webPassword;
    } else {
        Logger::notice("no saved web credentials, using default");
    }
    if(prefWeb.isKey("enabled")) {
        webCLIEnabled = prefWeb.getBool("enabled");
    }
    prefWeb.end();

    // add CLI client for web server
    cliClient = CLI.addClient(cliBuffer);

    if(webCLIEnabled) {
        enableWebCLI();
    } else {
        disableWebCLI();
    }

  return true;
}

boolean Network::setWebAuth(char* id, char* password) {
    webID = id;
    webPassword = password;
    // Save web CLI credentials
    prefWeb.begin(WEB_AUTH_KEY);
    prefWeb.putString("id", id);
    prefWeb.putString("password", password);
    prefWeb.end();
    return true;
}

boolean Network::webCLIEnabled = true;
boolean Network::enableWebCLI() {

    // clear server routes
    server.reset();

    // add route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        Logger::notice("incoming request for CLI page");
        if(!request->authenticate(webID.c_str(), webPassword.c_str())) {
            Logger::notice("requesting authentication");
            return request->requestAuthentication();
        }
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    // add route for command execution
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        AsyncWebParameter* cmd;
        if(!request->authenticate(webID.c_str(), webPassword.c_str()))
            return request->requestAuthentication();
        if(request->hasParam("cmd", true)) {
            // get cmd parameter and value
            AsyncWebParameter* cmdParm = request->getParam("cmd", true);
            String cmdString = request->urlDecode(cmdParm->value());
            Logger::notice(fmt("WiFi CLI - request: '%s'", cmdString));

            // prepare buffer for CLI processing
            cliBuffer.setInput(fmt("%s\n", (char*)cmdString.c_str()));
            cliClient->echo(false);

            // process the command
            CLI.process();

            char* resp = cliBuffer.getOutput();
            request->send(200, "text/plain", resp);
            cliBuffer.reset();
            
            Logger::verbose(fmt("WiFi CLI - response: '%s'", resp));
        }
    });

    Logger::notice("enabled Web CLI");
    webCLIEnabled = true;
    prefWeb.begin(WEB_AUTH_KEY);
    prefWeb.putBool("enabled", webCLIEnabled);
    prefWeb.end();
    return true;
}

boolean Network::disableWebCLI() {

    // clear server routes
    server.reset();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        Logger::notice("incoming request for CLI page, not available");
        request->send(503, "text/plain", "CLI Unavailable");
    });

    Logger::notice("disabled Web CLI");
    webCLIEnabled = false;
    prefWeb.begin(WEB_AUTH_KEY);
    prefWeb.putBool("enabled", webCLIEnabled);
    prefWeb.end();
    return true;
}

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
            if(!MDNS.begin(cliHost)) {
                Logger::error("unable to start mDNS");
            } else {
                MDNS.addService("http", "tcp", 80);
                Logger::notice("mDNS started");
            }
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
            hostIP = String(fmt("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]));
            Logger::notice(fmt("Obtained IP address: %s", hostIP));

            // Save network credentials if changed
            prefWiFi.begin(WIFI_KEY);
            Logger::verbose(fmt("checking ssid; stored: '%s' current: '%s'", prefWiFi.getString("ssid").c_str(), _ssid));
            if(strcmp(prefWiFi.getString("ssid").c_str(), _ssid) != 0 ||
                strcmp(prefWiFi.getString("password").c_str(), _password) != 0) {
                    Logger::verbose(fmt("saving credentials for %s", _ssid));
                    prefWiFi.putString("ssid", _ssid);
                    prefWiFi.putString("password", _password);
            } else {
                    Logger::verbose(fmt("wifi credentials unchanged for %s", _ssid));
            }
            prefWiFi.end();

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

            Logger::verbose("Starting Web CLI");
            server.begin();
            Logger::notice("Started Web CLI");

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
        // get WiFi credentials
        Logger::verbose("loading saved WiFi credentials");
        prefWiFi.begin(WIFI_KEY);
        String pSSID = prefWiFi.getString("ssid");
        String pPASS = prefWiFi.getString("password");
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

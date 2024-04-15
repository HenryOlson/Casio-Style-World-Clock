/*
 * top level (setup and loop) for Casio-style world clock
 * on LilyGo T-Display-S3
 *
 * hpro 2024-04-11
 * 
 */
#include <Logger.h>
#include <esp_adc_cal.h>
#include "pin_config.h"
#include "Network.h"
#include "CasioClock.hpp"
#include "CLI.hpp"
#include "Voltage.hpp"

// change settings button (top)
#define SET_BUTTON PIN_BUTTON_1
// change mode button (bottom)
#define MODE_BUTTON PIN_BUTTON_2
// pin high forces lcd on when on battery
#define LCD_ON_BATT PIN_POWER_ON
// time in seconds to keep LCD on when activated on battery
#define LCD_ON_BATT_TIME 15
// time in seconds to wait until going into low power mode when on battery
#define POWER_DOWN_DELAY 60

/*
 * Singleton(s)
 */
CasioClock theClock;
Voltage volts;

/*
 * support / utility functions
*/
// formatter for logging
char* fmt(const char* fmt, ...) {
    static char buf[200];
    va_list va;
    va_start (va, fmt);
    vsprintf (buf, fmt, va);
    va_end (va);
    return buf;
}

// log formatting and output
void clockLogger(Logger::Level level, const char* module, const char* message) {
  static char buf[25];

  // timstamp
  sprintf(buf, "[%8ld]", millis());
  Serial.print(buf);

  // log level
  sprintf(buf, ":[%-7s] ", Logger::asString(level));
  Serial.print(buf);

  // module
  if (strlen(module) > 0) {
      Serial.print(module);
      Serial.print(" ");
  }

  // message
  Serial.println(message);

}

/*
 * Power Management
 */
// configure wake on mode button
void wakeButton() {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    gpio_wakeup_enable(GPIO_NUM_14, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_14, 0);
}

// configure wake on timer
void wakeSeconds(int sleepSeconds) {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000);
}

// enter light sleep mode
void lightSleep() {
    // display off and notification
    theClock.setDisplay(false);
    Logger::notice("going to sleep");
    Serial.flush();
    delay(1);

    // sleep
    esp_light_sleep_start();

    // restore default wakeup configuration
    wakeButton();

    // display on and updated
    theClock.forceRefresh = true;
    theClock.setDisplay(true);

    Logger::notice("awake");
}

boolean powerStateHigh = true;
void powerHigh() {
    if(!powerStateHigh) {
        setCpuFrequencyMhz(240);
        Network::initWiFi();
        theClock.ntpOn();
        theClock.setDisplay(true);
        powerStateHigh = true;
        Logger::notice(fmt("power state is now %s", powerStateHigh ? "high" : "low"));
    }
}

void powerLow() {
    if(powerStateHigh) {
        theClock.ntpOff();
        Network::disconnect();
        theClock.setDisplay(false);
        setCpuFrequencyMhz(10);
        powerStateHigh = false;
        Logger::notice(fmt("power state is now %s", powerStateHigh ? "high" : "low"));
    }
}

void cpuStats() {
  Logger::notice(fmt("CPU Freq = %d MHz", getCpuFrequencyMhz()));
  Logger::notice(fmt("XTAL Freq = %d MHz", getXtalFrequencyMhz()));
  Logger::notice(fmt("APB Freq = %d MHz", getApbFrequency()/1000000));
}

/*
 * CLI Commands
 */
// update the time from the network service
int cliNtpUpdate(int argc = 0, char** argv = NULL) {
        Logger::notice("checking network time");
        theClock.update();
        delay(20);
        return 0;
}

// show the device uptime
int cliUptime(int argc = 0, char** argv = NULL) {
    // seconds
    int upTime = millis() / 1000;
    int secs = upTime % 60;

    // minutes
    upTime = upTime / 60;
    int mins = upTime % 60;

    // hrs
    upTime = upTime / 60;
    int hrs = upTime % 60;

    // days
    int days = upTime / 24;

    cli.printf("uptime %dd %02d:%02d:%02d", days, hrs, mins, secs);
    return 0;
}

// show the display update frames per second (loop counter)
int fps = 0;
int cliFPS(int argc = 0, char** argv = NULL) {
    cli.printf("%d FPS", fps);
    return 0;
}

// set the mode (without args, advances to next like the mode button)
const char* setModeHelp = "\
Usage: mode [lock|map|prime|color]\r\n\
    lock  - disable the set button\r\n\
    map   - change the map time\r\n\
    prime - change the timezone for the large display\r\n\
    color - the display color\r\n\
    next  - advance to next mode\r\n\
mode without arguments advances to next mode"
;
int cliSetMode(int argc = 0, char* argv[] = NULL) {
    return theClock.setMode(argc, argv);
}

// advance to the next setting in the current mode
int cliNextSetting(int argc = 0, char** argv = NULL) {
    return theClock.nextSetting(argc, argv);
}

// set clock time zone directly
int cliSetLocation(int argc = 0, char** argv = NULL) {
    int stat = 0;
    if(argc == 3) {
        String code = String(argv[2]);
        code.toUpperCase();
        if(strcmp(argv[1], "map") == 0) {
            stat = (theClock.changeMapLocation((char*)code.c_str()) ? 0 : 1);
            if(stat != 0)
                cli.printf("invalid location code '%s'", argv[2]);
        } else if(strcmp(argv[1], "prime") == 0) {
            stat = (theClock.changePrimeLocation((char*)code.c_str()) ? 0 : 1);
            if(stat != 0)
                cli.printf("invalid location code '%s'", argv[2]);
        } else {
            cli.printf("invalid clock name '%s'", argv[1]);
            stat = 2;
        }
    } else {
        cli.printf("invalid argument count %d", argc);
        return CLI_ARG_COUNT;
    }
    return stat;
}

int cliSetFormat(int argc = 0, char* argv[] = NULL) {
    return theClock.changeTimeFormat(argc, argv);
}

// reboot the device
int cliReboot(int argc, char* argv[]) {
    cli.printf("Rebooting ...");
    ESP.restart();
    return 0;           // keep the compiler happy
}

// (re)start the wifi connection, either with current credentials or with new
const char* startWiFiHelp = "\
Usage: wifi [ssid pass]\r\n\
    ssid  - ssid to connect to\r\n\
    pass  - wifi password\r\n\
wifi without arguments connects with current saved settings"
;
int cliStartWiFi(int argc, char* argv[]) {
    switch(argc) {
        case 1:
            return (Network::initWiFi() ? 0 : 1);
            break;
        case 3:
            return (Network::initWiFi(argv[1], argv[2]) ? 0 : 2);
            break;
        default:
            cli.printf("ERROR: wifi - invalid argument count - %d", argc);
            return 3;
    }
    return 0;
}

// set the logging level
int cliSetLog(int argc, char* argv[]) {
    static Logger::Level lastLevel = Logger::Level::VERBOSE;
    switch(argc) {
        case 1:     // show log level
            cli.printf("log level %s", Logger::asString(Logger::getLogLevel()));
            break;
        case 2:     // on / off / level
            if(strcmp(argv[1], "level") == 0) {
                lastLevel = Logger::getLogLevel();
            } else if(strcmp(argv[1], "on") == 0) {
                Logger::setLogLevel(lastLevel);
            } else if(strcmp(argv[1], "off") == 0) {
                lastLevel = Logger::getLogLevel();
                Logger::setLogLevel(Logger::Level::SILENT);
            } else {
                cli.printf("invalid log command '%s'", argv[1]);
                return 2;
            }
            cli.printf("log level %s", Logger::asString(Logger::getLogLevel()));
            break;
        case 3:     // level <level>, e.g. level verbose
            if(strcmp(argv[1], "level") == 0) {
                if(strcmp(argv[2], "verbose") == 0) {
                    Logger::setLogLevel(Logger::Level::VERBOSE);
                } else if(strcmp(argv[2], "notice") == 0) {
                    Logger::setLogLevel(Logger::Level::NOTICE);
                } else if(strcmp(argv[2], "warning") == 0) {
                    Logger::setLogLevel(Logger::Level::WARNING);
                } else if(strcmp(argv[2], "error") == 0) {
                    Logger::setLogLevel(Logger::Level::ERROR);
                } else if(strcmp(argv[2], "fatal") == 0) {
                    Logger::setLogLevel(Logger::Level::FATAL);
                } else if(strcmp(argv[2], "silent") == 0) {
                    Logger::setLogLevel(Logger::Level::SILENT);
                } else {
                    cli.printf("invalid log level '%s'", argv[2]);
                    return 1;
                }
                lastLevel = Logger::getLogLevel();
            } else {
                cli.printf("invalid log command '%s'", argv[1]);
                return 2;
            }
            cli.printf("log level %s", Logger::asString(Logger::getLogLevel()));
            break;
        default:
            cli.printf("Invalid log arguments");
            return 3;
    }
    return 0;
}

int cliScreen(int argc = 0, char* argv[] = NULL) {
    switch(argc) {
        case 1:
            // show the display setting
            theClock.setDisplay();
            break;
        case 2:
            // set the display on / off
            if(strcmp(argv[1], "on") == 0) {
                // ToDo: temporary
                theClock.ntpOn();
                theClock.setDisplay(true);
            } else if(strcmp(argv[1], "off") == 0) {
                theClock.setDisplay(false);
                // ToDo: temporary
                theClock.ntpOff();
            } else {
                Logger::error(fmt("invalid %s option '%s'", argv[0], argv[1]));
                return 1;
            }
            break;
        default:
            Logger::error(fmt("invalid %s argument count", argv[0]));
            return 2;
    }
    return 0;
}

// go to sleep
int cliSleep(int argc = 0, char* argv[] = NULL) {
    lightSleep();
    return 0;
}

// configure wake triggers
int cliWake(int argc, char* argv[]) {
    int sleepSeconds;
    switch(argc) {
        case 1:
            // wake now not implemented
            break;
        case 2:
            // wake button
            if(strcmp(argv[1], "button") == 0) {
                wakeButton();
                return 0;
            } else
                break;
        case 3:
            // wake after <seconds>
            if(strcmp(argv[1], "after") == 0) {
                wakeSeconds(atoi(argv[2]));
                return 0;
            } else
                break;
    }
    cli.printf("invalid wake command");
    return 1;
}

/*
// If the battery is not connected, the ADC detects the charging voltage of TP4056, which is inaccurate.
// Can judge whether it is greater than 4300mV. If it is less than this value, it means the battery exists.
// my observation is that with no battery I get 4768-4778
// with battery up to 4814 (fully charged), so no way do detect if line power is available
if (v1 > 4300) {
    cli.printf("battery not present");
}
*/
// monitor / show battery voltage
int cliBattery(int argc = 0, char* argv[] = NULL) {
    switch (argc) {
        case 1:
            cli.printf("voltage: %dmv", volts.read());
            return 0;
        case 2:
            if(strcmp(argv[1], "volts") == 0) {
                cli.printf("voltage: %dmv", volts.read());
                return 0;
            }
            break;
        case 3:
            if(strcmp(argv[1], "monitor") == 0) {
                if(strcmp(argv[2], "on") == 0) {
                    volts.monitorOn();
                    return 0;
                } else if(strcmp(argv[2], "off") == 0) {
                    volts.monitorOff();
                    return 0;
                } else if(strcmp(argv[2], "show") == 0) {
                    volts.dump();
                    return 0;
                } else if(strcmp(argv[2], "clear") == 0) {
                    volts.clearHistory();
                    return 0;
                }
            }
            break;
    }
    cli.printf("invalid %s command", argv[0]);
    return 1;
}

// show / set CPU speed
// assuming 40Mhz XTAL
int cliCPU(int argc = 0, char* argv[] = NULL) {
    switch(argc) {
        case 1:
            break;
        case 2:
            if(strcmp(argv[1], "show") == 0) {
            } else if(strcmp(argv[1], "fast") == 0) {
                setCpuFrequencyMhz(240);
            } else if(strcmp(argv[1], "slow") == 0) {
                setCpuFrequencyMhz(10);
            } else {
                int freq = atoi(argv[1]);
                switch(freq) {
                    case 240:
                    case 160:
                    case 80:
                    case 40:
                    case 20:
                    case 10:
                        setCpuFrequencyMhz(freq);
                        break;
                    default:
                        cli.printf("invalid cpu speed '%s'", argv[1]);
                        return 1;
                }
            }
            break;
        default:
            cli.printf("invalid cpu command");
            return 2;
    }
    cli.printf("CPU freq %d MHz", getCpuFrequencyMhz());
    return 0;
}

int cliPower(int argc = 0, char* argv[] = NULL) {
    switch(argc) {
        case 1:
            cli.printf("power state is %s", powerStateHigh ? "high" : "low");
            break;
        case 2:
            if(strcmp(argv[1], "low") == 0) {
                powerLow();
            } else if(strcmp(argv[1], "high") == 0) {
                powerHigh();
            } else {
                cli.printf("invalid power command %s", argv[1]);
                return 1;
            }
            break;
        default:
            cli.printf("invalid power command");
            return 2;
    }
    return 0;
}

/*
 * Arduino core
 */
void setup() {

    Serial.begin(115200); //  enable log / debug output

    // use the following line to delay startup until serial is connected
    // can be useful for development, off for deployment
    //while(!Serial) continue;

    // Configure logger
    //Logger::setLogLevel(Logger::VERBOSE);
    Logger::setLogLevel(Logger::NOTICE);
    Logger::setOutputFunction(clockLogger);

    Logger::notice("'Casio' clock setup");
    Logger::notice(fmt("LVGL v%s", theClock.lvglVersion().c_str()));

    cpuStats();

    // configure clock buttons
    pinMode(SET_BUTTON, INPUT_PULLUP);
    pinMode(MODE_BUTTON, INPUT_PULLUP);

    // set default wakeup trigger
    wakeButton();

    // initialize the clock logic and display
    theClock.init();
 
    // start WiFi
    if(!Network::initWiFi()) {
        Logger::notice("WiFi not started, please enter SSID / password");
    }

    // configure CLI commands
    cli.add("log", cliSetLog, "logging control (on, off, level ..)");
    cli.add("mode", cliSetMode, "change modes (lock,map,prime,format,color), default next", setModeHelp);
    cli.add("next", cliNextSetting, "next setting in current mode");
    cli.add("location", cliSetLocation, "location [ map | prime ] <code>");
    cli.add("format", cliSetFormat, "format [ show | 12 | 24 ]");
    cli.add("wifi", cliStartWiFi, "(re)connect WiFi", startWiFiHelp);
    cli.add("screen", cliScreen, "screen [ on | off ]");
    cli.add("wake", cliWake, "wake [ button | after <seconds> ]");
    cli.add("sleep", cliSleep, "sleep");
    cli.add("battery", cliBattery, "battery [ volts | show | monitor ( on | off | clear) ]");
    cli.add("power", cliPower, "power [ low | high ]");
    cli.add("cpu", cliCPU, "cpu [ show | fast | slow ]");
    cli.add("update", cliNtpUpdate, "update time from network (NTP)");
    cli.add("fps", cliFPS, "show display update FPS");
    cli.add("uptime", cliUptime, "show uptime");
    cli.add("reboot", cliReboot, "reboot the clock");
    cli.begin("clock> ");

    Logger::notice("Setup done");

}

int lastSecond = -1;
int frameCount = 0;
int lcdStart = 0;
int batStart = 0;
void smallloop() {
    // process Serial command line input
    cli.read();
}

void loop() {

    // GUI background
    theClock.idle();

    // Check for button press on MODE_BUTTON
    if (digitalRead(MODE_BUTTON) == LOW) {
        digitalWrite(LCD_ON_BATT, HIGH);
        lcdStart = millis();
        delay(300);  // Debounce delay
        theClock.setMode();
    }
    if(lcdStart != 0 && ((millis() - lcdStart)/1000) > LCD_ON_BATT_TIME) {
        digitalWrite(LCD_ON_BATT, LOW);
        lcdStart = 0;
    }

    // Check for button press on SET_BUTTON
    if (digitalRead(SET_BUTTON) == LOW) {
        lcdStart = millis();
        delay(300);  // Debounce delay
        theClock.nextSetting();
    }

    // refresh display
    int currentSecond = theClock.refresh();

    // once per second
    if(currentSecond != lastSecond) {

        // track battery voltage
        volts.monitor();

        /*
         * the following sleep logic does not work as intended:
         *
         * without the following code
         * when the device is unplugged, the display goes off
         *   but the cpu still runs (monitoring continues)
         *
         * with this code
         *   the volts reading does go to zero
         *   when the device goes into light sleep, it won't come back until plugged in
         *   when the digitalWrite and wakeButton code are employed,
         *     when the device comes back on power, USB serial does not come back
         */
/*
        // go to sleep if on battery
        if(volts.isZero()) {
            digitalWrite(PIN_POWER_ON, HIGH);
            wakeButton();
            lightSleep();
        }
*/
        // reduce power if on battery - this works,
        // but only looks to get about 5 hours on battery (~80ma)
        if(volts.isZero()) {
            if(batStart = 0) {
                batStart = millis();
            } else if(((millis() - batStart)/1000) > POWER_DOWN_DELAY) {
                powerLow();
                batStart = 0;
            }
        } else {
            powerHigh();
            batStart = 0;
        }

        // track frame rate
        fps = frameCount;
        frameCount = 0;

        lastSecond = currentSecond;
    } else {
        frameCount++;
    }

    // update from network time once per 15 minutes
    // complaints from WiFiUDP when in low power mode
    if(powerStateHigh && ((millis() % 900000) < 20)) {
        cliNtpUpdate();
    }
    
    // process Serial command line input
    cli.read();

}

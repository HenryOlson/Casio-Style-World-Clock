/*
 * top level (setup and loop) for Casio-style world clock
 * on LilyGo T-Display-S3
 *
 * hpro 2024-04-11
 * 
 */
#include <Logger.h>
#include <esp_adc_cal.h>
#include "CLI.h"
#include "pin_config.h"
#include "Network.h"
#include "CasioClock.hpp"
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

  // timestamp
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
CLI_COMMAND(cliNtpUpdate) {
        if(!theClock.update()) {
            return 1;
        }
        return 0;
}

// show the device uptime
CLI_COMMAND(cliUptime) {
    // seconds
    int upTime = millis() / 1000;
    int secs = upTime % 60;

    // minutes
    upTime = upTime / 60;
    int mins = upTime % 60;

    // hrs
    upTime = upTime / 60;
    int hrs = upTime % 24;

    // days
    int days = upTime / 24;

    dev->printf("uptime %dd %02d:%02d:%02d\r\n", days, hrs, mins, secs);
    return 0;
}

// show the display update frames per second (loop counter)
int fps = 0;
CLI_COMMAND(cliFPS) {
    dev->printf("%d FPS\r\n", fps);
    return 0;
}

// set the mode (without args, advances to next like the mode button)
const char* setModeHelp = "\
    lock  - disable the set button\r\n\
    map   - change the map time\r\n\
    prime - change the timezone for the large display\r\n\
    color - the display color\r\n\
    next  - advance to next mode\r\n\
mode without arguments advances to next mode"
;
CLI_COMMAND(cliSetMode) {
    // Set the current action mode if a parmeter was provided
    int theMode = theClock.getMode();
    switch(argc) {
        case 1:
            // advance to the next mode
            theClock.setMode(-1);
            break;
        case 2:
            if(strcmp(argv[1], "lock") == 0) {
                theMode = 0;
            } else if(strcmp(argv[1], "map") == 0) {
                theMode = 1;
            } else if(strcmp(argv[1], "prime") == 0) {
                theMode = 2;
            } else if(strcmp(argv[1], "format") == 0) {
                theMode = 3;
            } else if(strcmp(argv[1], "color") == 0) {
                theMode = 4;
            } else if(strcmp(argv[1], "next") == 0) {
                theMode = ++theMode % 5;
            } else {
                Logger::error(fmt("invalid mode'%s'", argv[1]));
                return 1;
            }
            break;
    }
    theClock.setMode(theMode);
    return 0;
}

// advance to the next setting in the current mode
CLI_COMMAND(cliNextSetting) {
    theClock.nextSetting();
    return 0;
}

// set clock time zone directly
CLI_COMMAND(cliSetLocation) {
    int stat = 0;
    if(argc == 3) {
        String code = String(argv[2]);
        code.toUpperCase();
        if(strcmp(argv[1], "map") == 0) {
            stat = (theClock.changeMapLocation((char*)code.c_str()) ? 0 : 1);
            if(stat != 0)
                dev->printf("invalid location code '%s'\r\n", argv[2]);
        } else if(strcmp(argv[1], "prime") == 0) {
            stat = (theClock.changePrimeLocation((char*)code.c_str()) ? 0 : 1);
            if(stat != 0)
                dev->printf("invalid location code '%s'\r\n", argv[2]);
        } else {
            dev->printf("invalid clock name '%s'\r\n", argv[1]);
            stat = 2;
        }
    } else {
        dev->printf("invalid argument count %d\r\n", argc);
        return 1000;
    }
    return stat;
}

CLI_COMMAND(cliSetFormat) {
    int rval = 0;
    switch(argc) {
        case 0:
            // null arguments - button push - advance to next setting
            theClock.setTimeFormat(0);
            break;
        case 1:
            // no command arguments - just show the setting
            break;
        case 2:
            // 2 arguments - show or set the time format
            if(strcmp(argv[1], "show") == 0) {
                    // fall through to show
            } else {
                int which = atoi(argv[1]);
                switch(which) {
                    case 12:
                    case 24:
                        theClock.setTimeFormat(which);
                        break;
                    default:
                        Logger::error(fmt("%s: invalid time format %s", argv[0], argv[1]));
                        return 1;
                }
            }
            break;
        default:
            // extra arguments
            Logger::error(fmt("%s: too many arguments (%d)", argv[0], argc));
            return 2;
    }
    // show the format
    Logger::notice(fmt("time format %d", theClock.getTimeFormat()));
    return 0;
}

// reboot the device
CLI_COMMAND(cliReboot) {
    dev->printf("Rebooting ...");
    ESP.restart();
    return 0;           // keep the compiler happy
}

// (re)start the wifi connection, either with current credentials or with new
const char* startWiFiHelp = "\
    ssid  - ssid to connect to\r\n\
    pass  - wifi password\r\n\
wifi without arguments connects with current saved settings"
;
CLI_COMMAND(cliStartWiFi) {
    switch(argc) {
        case 1:
            return (Network::initWiFi() ? 0 : 1);
            break;
        case 3:
            return (Network::initWiFi(argv[1], argv[2]) ? 0 : 2);
            break;
        default:
            dev->printf("ERROR: wifi - invalid argument count - %d\r\n", argc);
            return 3;
    }
    return 0;
}

// set the logging level
CLI_COMMAND(cliSetLog) {
    static Logger::Level lastLevel = Logger::Level::VERBOSE;
    switch(argc) {
        case 1:     // show log level
            dev->printf("log level %s\r\n", Logger::asString(Logger::getLogLevel()));
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
                dev->printf("invalid log command '%s'\r\n", argv[1]);
                return 2;
            }
            dev->printf("log level %s\r\n", Logger::asString(Logger::getLogLevel()));
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
                    dev->printf("invalid log level '%s'\r\n", argv[2]);
                    return 1;
                }
                lastLevel = Logger::getLogLevel();
            } else {
                dev->printf("invalid log command '%s'\r\n", argv[1]);
                return 2;
            }
            dev->printf("log level %s\r\n", Logger::asString(Logger::getLogLevel()));
            break;
        default:
            dev->printf("Invalid log arguments\r\n");
            return 3;
    }
    return 0;
}

CLI_COMMAND(cliScreen) {
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
CLI_COMMAND(cliSleep) {
    lightSleep();
    return 0;
}

// configure wake triggers
CLI_COMMAND(cliWake) {
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
    dev->printf("invalid wake command\r\n");
    return 1;
}

/*
// If the battery is not connected, the ADC detects the charging voltage of TP4056, which is inaccurate.
// Can judge whether it is greater than 4300mV. If it is less than this value, it means the battery exists.
// my observation is that with no battery I get 4768-4778
// with battery up to 4814 (fully charged), so no way do detect if line power is available
if (v1 > 4300) {
    dev->printf("battery not present\r\n");
}
*/
// monitor / show battery voltage
CLI_COMMAND(cliBattery) {
    switch (argc) {
        case 1:
            dev->printf("voltage: %dmv\r\n", volts.read());
            return 0;
        case 2:
            if(strcmp(argv[1], "volts") == 0) {
                dev->printf("voltage: %dmv\r\n", volts.read());
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
    dev->printf("invalid %s command\r\n", argv[0]);
    return 1;
}

// show / set CPU speed
// assuming 40Mhz XTAL
CLI_COMMAND(cliCPU) {
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
                        dev->printf("invalid cpu speed '%s'\r\n", argv[1]);
                        return 1;
                }
            }
            break;
        default:
            dev->printf("invalid cpu command\r\n");
            return 2;
    }
    dev->printf("CPU freq %d MHz\r\n", getCpuFrequencyMhz());
    return 0;
}

CLI_COMMAND(cliPower) {
    switch(argc) {
        case 1:
            dev->printf("power state is %s\r\n", powerStateHigh ? "high" : "low");
            break;
        case 2:
            if(strcmp(argv[1], "low") == 0) {
                powerLow();
            } else if(strcmp(argv[1], "high") == 0) {
                powerHigh();
            } else {
                dev->printf("invalid power command %s\r\n", argv[1]);
                return 1;
            }
            break;
        default:
            dev->printf("invalid power command\r\n");
            return 2;
    }
    return 0;
}

CLI_COMMAND(cliWelcome) {
    dev->println("Welcome to the clock CLI.");
    dev->println("Type 'help' to list commands.");
    dev->println();
    return 0;
}

CLI_COMMAND(cliWeb) {
    switch(argc) {
        case 1:
            // status?
            return 0;
        case 2:
            // on / off
            if(strcmp(argv[1], "on") == 0) {
                Network::beginWebCLI();
                return 0;
            } else if(strcmp(argv[1], "off") == 0) {
                Network::endWebCLI();
                return 0;
            }
            break;
        case 4:
            // auth id password
            if(strcmp(argv[1], "auth") == 0) {
                Network::setWebAuth(argv[2], argv[3]);
                return 0;
            }

    }
    dev->printf("%s: invalid command line", argv[0]);
    return 1;
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

    // configure Web CLI
    Network::initWebCLI("clock", "clock> ");

    // configure CLI commands
    CLI.setDefaultPrompt("clock> ");
    //CLI.onConnect(cliOnConnect);
    CLI.addCommand("log", cliSetLog, "control logging", "log (on, off, level {level}");
    CLI.addCommand("mode", cliSetMode, "change modes", "mode (lock,map,prime,format,color), default next", setModeHelp);
    CLI.addCommand("next", cliNextSetting, "choose next setting in current mode");
    CLI.addCommand("location", cliSetLocation, "set location for clock", "location [ map | prime ] {code}");
    CLI.addCommand("format", cliSetFormat, "set time format", "format [ show | 12 | 24 ]");
    CLI.addCommand("wifi", cliStartWiFi, "(re)connect WiFi", "wifi [ssid pwd]", startWiFiHelp);
    CLI.addCommand("screen", cliScreen, "turn screen on or off", "screen [ on | off ]");
    CLI.addCommand("wake", cliWake, "set wakeup trigger", "wake [ button | after {seconds} ]");
    CLI.addCommand("sleep", cliSleep, "enter light sleep", "sleep");
    CLI.addCommand("battery", cliBattery, "show or monitor battery voltage", "battery [ volts | show | monitor ( on | off | clear) ]");
    CLI.addCommand("power", cliPower, "show or set power level", "power [ low | high ]");
    CLI.addCommand("cpu", cliCPU, "show or set cpu speed", "cpu [ show | fast | slow ]");
    CLI.addCommand("update", cliNtpUpdate, "update time from network (NTP)");
    CLI.addCommand("fps", cliFPS, "show display update FPS");
    CLI.addCommand("uptime", cliUptime, "show uptime");
    CLI.addCommand("web", cliWeb, "control web CLI", "web [ on | off | auth {id} {password} ]");
    CLI.addCommand("reboot", cliReboot, "reboot the clock");
    CLI.onConnect(cliWelcome);
    CLI.addClient(Serial);

    Logger::notice("Setup done");

}

int lastSecond = -1;
int frameCount = 0;
int lcdStart = 0;
int batStart = 0;
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
        theClock.update();
    }
    
    // process command line input
    CLI.process();

}

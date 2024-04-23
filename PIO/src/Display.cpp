/*
 * ui management for the CasioClock
 */
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <ui.h>
#include <Logger.h>
#include "pin_config.h"
#include "CasioClock.hpp"
#include "Display.hpp"

// required coupling
extern CasioClock theClock;
extern char* fmt(const char* fmt, ...);

/* Set to screen resolution of T-Display-S3 */
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 170;

/*
 * public - except for the tft, not clear that these should be public
  *         however, have not yet been able to make that work
 */
TFT_eSPI tft; /* TFT instance, should be a singleton, but access not needed outside this module */

// Display flush callback
// NB: should be private
void displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

// NB: compiles but causes crash if this is made private
lv_obj_t *ui_img_loc;
 
/*
 * private
 */
Location* mapLocation;
Location* primeLocation;
boolean displayOn = true;

lv_obj_t *ui_img_map_bg;
lv_obj_t *ui_img_world_time;
lv_obj_t *ui_img_local_time;

Display::Display(Location* map, Location* prime) {
    mapLocation = map;          // GMT
    primeLocation = prime;          // GMT -> Local (LOC)
    timeFormat = 12;
}

// Display seconds with two digits
void Display::refreshSeconds(int currentSeconds) {
        char secondsText[10];
        sprintf(secondsText, "%02d", currentSeconds);
        // ToDo: the name of this label seems wrong, seconds are in the 'B' (Prime) clock
        lv_label_set_text(ui_Label_wt_sec_a, secondsText);
}

void Display::refreshMapTime() {
    Logger::verbose("Entering refreshMapTime");

    struct tm* timeInfo = getTZTime(mapLocation->tz);

    String time = getTimeString(timeInfo);
    String ampm = getAmPmString(timeInfo);
    Logger::verbose("Got MapTime");
    String code = mapLocation->locationCode;

    // show the time
    Logger::verbose(fmt("Showing time ", time.c_str()));
    lv_label_set_text(ui_Label_lt_a, time.c_str());
    lv_label_set_text(ui_Label_lt_ampm, ampm.c_str());

    // Set the new coordinates for ui_img_loc based on the current Map time
    Logger::verbose(fmt("Setting map coordinates: %d,%d",
        mapLocation->locX, mapLocation->locY));
    lv_obj_set_x(ui_img_loc, mapLocation->locX);
    lv_obj_set_y(ui_img_loc, mapLocation->locY);

    // show the location code
    lv_label_set_text(ui_Label_cc_a, code.c_str());

    Logger::verbose("updated Map time labels");
    Logger::verbose(fmt("%s: %s %s", code, time, ampm));
}

void Display::refreshPrimeTime() {
    Logger::verbose("entering refreshPrimeTime");

    struct tm* timeInfo = getTZTime(primeLocation->tz);

    String time = getTimeString(timeInfo);
    String ampm = getAmPmString(timeInfo);
    String zone = getTZOffsetString(timeInfo);
    String code = primeLocation->locationCode;
    Logger::verbose("got time strings");

    // Update LVGL labels with the time information
    refreshSeconds(theClock.getSeconds());

    lv_label_set_text(ui_Label_wt_a, time.c_str());
    lv_label_set_text(ui_Label_wt_ampm, ampm.c_str());
    lv_label_set_text(ui_Label_cc, code.c_str());
    lv_label_set_text(ui_Label_gmt, zone.c_str());

    Logger::verbose("updated Prime time labels");
    Logger::verbose(fmt("%s: %s %s", code, time, ampm));

}

void Display::setMapLocation(Location* newLocation) {
    mapLocation = newLocation;
}

void Display::setPrimeLocation(Location* newLocation) {
    primeLocation = newLocation;
}

void Display::init() {
    static lv_color_t buf[screenWidth * screenHeight / 10];
    static lv_disp_draw_buf_t draw_buf;

    forceRefresh = false;

    lv_init();

    tft.begin();        /* TFT init */
    tft.setRotation(3); /* Landscape orientation, flipped */

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    // Initialize the display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = displayFlush;

    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    ui_init();

    // Enable the location animation with a delay of 2000 milliseconds (2 seconds)
    imgloc_Animation(ui_img_loc, 2000);

    idle();

    // backlight control
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);
    pinMode(PIN_POWER_ON, OUTPUT);

    displayOn = true;
}


boolean Display::changeBG() {
    static boolean bgVisible = false;
    delay(5);
    // Code for changeBG
    if (!bgVisible) {
        Logger::verbose("Executing changeBG: Showing elements");
        lv_obj_clear_flag(ui_img_local_time_blue, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_img_map_bg_blue, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_img_world_time_blue, LV_OBJ_FLAG_HIDDEN);
    } else {
        Logger::verbose("Executing changeBG: Hiding elements");
        lv_obj_add_flag(ui_img_local_time_blue, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_img_map_bg_blue, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_img_world_time_blue, LV_OBJ_FLAG_HIDDEN);
    }

    // Toggle the visibility state
    bgVisible = !bgVisible;
    delay(5);
    return bgVisible;
}


#define OVERWRITE 1
struct tm* Display::getTZTime(const char* tz) {

    setenv("TZ", tz, OVERWRITE);
    tzset();

    time_t currtime;
    currtime = theClock.getEpochTime();
    //currtime = time(NULL);
    struct tm* timeinfo = localtime ( &currtime );

    return timeinfo;
}

String Display::getTimeString(struct tm* timeInfo) {

    // Format the time without leading zero for hour
    char formattedTime[9];                                             // HH:MM\0
    if(timeFormat == 12) {
        strftime(formattedTime, sizeof(formattedTime), "%l:%M", timeInfo); // 12-hour format without leading zero
    } else {
        strftime(formattedTime, sizeof(formattedTime), "%k:%M", timeInfo); // 24-hour format without leading zero
    }

    return String(formattedTime);
}

String Display::getAmPmString(struct tm* timeInfo) {

    // Format AM/PM
    if(timeFormat == 12) {
        char ampm[3];
        strftime(ampm, sizeof(ampm), "%p", timeInfo);
        return String(ampm);
    } else {
        return String("  ");
    }

}

// Format time zone offset
String Display::getTZOffsetString(struct tm* timeInfo) {

    // get the offset (as a string, e.g. +0530)
    char tzo[6];
    strftime(tzo, sizeof(tzo), "%z", timeInfo);

    // convert to hours and minutes
    int tzhm = atoi(tzo);
    int tzm = tzhm % 100;
    int tzh = (tzhm-tzm) / 100;

    // format for display with leading sign and : separator (e.g. +5:30)
    char ftzo[7];
    if(tzm == 0) {
        sprintf(ftzo, "%+d", tzh);
    } else {
        sprintf(ftzo, "%+d:%d", tzh, tzm);
    }

    return String(ftzo);
}

// NB: optimized refresh
//  Depends on being called more than once per second
int Display::refresh() {
    static int refreshTime = 0;
    int currentSeconds = theClock.getSeconds();
    if(!displayOn) {
        return currentSeconds;
    }

    if(forceRefresh || (currentSeconds == 0 && refreshTime != theClock.getEpochTime())) {
        forceRefresh = false;
        Logger::verbose("Full display refresh");
        refreshTime = theClock.getEpochTime();
        refreshMapTime();
        refreshPrimeTime();
    } else {
        refreshSeconds(currentSeconds);
    }
    return currentSeconds;
}

boolean Display::setDisplay(int turnOn) {
    switch(turnOn) {
        case -1:
            break;
        case true:
            if(!displayOn) {
                tft.init();
                tft.writecommand(ST7789_DISPON); //turn on display
                digitalWrite(LCD_BL, HIGH); //turn on lcd backlight
                //digitalWrite(PIN_POWER_ON, HIGH);
                displayOn = true;
            }
            break;
        case false:
            if(displayOn) {
                tft.writecommand(ST7789_DISPOFF); //turn off lcd display
                digitalWrite(LCD_BL, LOW); //turn off lcd backlight
                //digitalWrite(PIN_POWER_ON, LOW);
                displayOn = false;
            }
            break;
        default:
            Logger::error("invalid display command");
    }
    Logger::verbose(fmt("display is %s", (displayOn ? "on" : "off")));
    return displayOn;
}

void Display::showMode(int mode) {
    // Set all visibility flags to LV_OBJ_FLAG_HIDDEN
    lv_obj_add_flag(ui_action_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_action_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_action_3, LV_OBJ_FLAG_HIDDEN);

    switch (mode) {
        case 0:
            Logger::notice("Mode is Lock");
            break;
        case 1:
            Logger::notice("Mode is Set Map Time");
            lv_obj_clear_flag(ui_action_1, LV_OBJ_FLAG_HIDDEN);
            break;
        case 2:
            Logger::notice("Mode is Set Prime Time");
            lv_obj_clear_flag(ui_action_2, LV_OBJ_FLAG_HIDDEN);
            break;
        case 3:
            Logger::notice("Mode is Set Time Format");
            // ToDo: should provide a visual indication
            //lv_obj_clear_flag(ui_action_3, LV_OBJ_FLAG_HIDDEN);
            break;
        case 4:
            Logger::notice("Mode is Set Color");
            lv_obj_clear_flag(ui_action_3, LV_OBJ_FLAG_HIDDEN);
            break;
    }
}

void Display::idle() {
    if(!displayOn)
        return;
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}

String Display::lvglVersion() {
    return String("") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
}

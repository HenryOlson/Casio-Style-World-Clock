#ifndef DISPLAY_H
#define DISPLAY_H
#include <lvgl.h>
#include "Locations.h"

class Display {

    public:

    // when true, do a full refresh on the next cycle
    boolean forceRefresh;
    // 12 or 24
    int timeFormat;

    // lifecycle
    Display(Location* mapLocation, Location* primeLocation);
    void init();
    void idle();
    int refresh();
    boolean setDisplay(int turnOn = -1);
    void refreshMapTime();
    void refreshPrimeTime();
    void showMode(int mode);
    // mutators
    void setMapLocation(Location* newLocation);
    void setPrimeLocation(Location* newLocation);
    boolean changeBG();
    // info
    String lvglVersion();

    private:

    Location* mapLocation;
    Location* primeLocation;
    boolean displayOn = true;

    lv_obj_t *ui_img_map_bg;
    lv_obj_t *ui_img_world_time;
    lv_obj_t *ui_img_local_time;

    struct tm* getTZTime(const char* tz);
    void refreshSeconds(int currentSeconds);
    String getTimeString(struct tm* timeInfo);
    String getAmPmString(struct tm* timeInfo);
    String getTZOffsetString(struct tm* timeInfo);

};
#endif

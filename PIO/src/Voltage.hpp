#include <Arduino.h>

#define MAX_HISTORY 200
#define RECORD_INTERVAL 5

class Voltage {
    public:
    Voltage();
    uint32_t read();
    boolean isZero();
    void monitorOn();
    void monitorOff();
    void clearHistory();
    void dump();
    boolean increasing();
    boolean decreasing();
    boolean monitor();

    private:
    int history[MAX_HISTORY];
    int hp;
    int monitoring;
    long lastread;
};
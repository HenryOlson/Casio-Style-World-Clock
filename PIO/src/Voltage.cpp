#include <esp_adc_cal.h>
#include "Voltage.hpp"
#include "pin_config.h"

Voltage::Voltage() {
    hp = 0;
}

#define READ_COUNT 3
uint32_t Voltage::read() {
    esp_adc_cal_characteristics_t adc_chars;

    // Get the internal calibration value of the chip
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    int reads = READ_COUNT;
    uint32_t raw = 0;
    while(reads--) {
        raw += analogRead(PIN_BAT_VOLT);
        //delay(1);
    }
    raw = raw / READ_COUNT;
    uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2; //The partial pressure is one-half

    return v1;
}

boolean Voltage::isZero() {
    return read() == 0;
}

void Voltage::monitorOn() {
    monitoring = true;
}

void Voltage::monitorOff(){
    monitoring = false;
}

void Voltage::clearHistory() {
    hp = 0;
}

void Voltage::dump() {
    char pbuf[100];
    int count = (hp > MAX_HISTORY ? MAX_HISTORY : hp);
    int start = ((hp % MAX_HISTORY) - count);
    start = start >= 0 ? start : MAX_HISTORY + start;
    sprintf(pbuf, "%d readings", count);
    Serial.println(pbuf);
    //sprintf(pbuf, "start at %d", start);
    //Serial.println(pbuf);
    for(int i=0; i<count; i++) {
        sprintf(pbuf, "%4d - %d", i, history[(i+start) % MAX_HISTORY]);
        Serial.println(pbuf);
    }
}

boolean Voltage::increasing(){
    if(monitor()) {
        // ToDo: calculations
        return false;
        //return true;
    } else {
        return false;
    }
}

boolean Voltage::decreasing(){
    if(monitor()) {
        // ToDo: calculations
        return false;
        //return true;
    } else {
        return false;
    }
}

boolean Voltage::monitor() {
    if(monitoring) {
        if(((millis() - lastread)/1000) >= RECORD_INTERVAL) {
            history[hp++ % MAX_HISTORY] = read();
            lastread = millis();
        }
        return true;
    }
    return false;
}

uint32_t history[MAX_HISTORY];
int hp;
int monitoring;
long lastread;

#pragma once

#ifndef thermometer_h
#define thermometer_h

/////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include <BLEScan.h>

/////////////////////////////////////////////////////////////////

#define THERMOMETERNAME "MJ_HT_V1"

/////////////////////////////////////////////////////////////////



class Thermometer {
    private:
        static Thermometer *thermometer;
        float temp;
        float humidity;
        int scanTime = 5; //In seconds
        BLEScan* pBLEScan;
        std::string deviceName;

        Thermometer();
        typedef void (*CallbackFunction)(float, float);
        CallbackFunction m_on_change = nullptr;


    public:
        static Thermometer *get();
        CallbackFunction getOnChange();
        void setOnChange(CallbackFunction callback);
        void loop();

};
/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////

#include "thermometer.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <SPI.h>

/////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        string deviceName = string(advertisedDevice.getName().c_str());

        if (string(THERMOMETERNAME) == deviceName)
        {
            auto payload = advertisedDevice.getPayload();
            float temp, humidity;
            auto id = payload[18];

            if (id == 0x0d)
            {
                temp = (((int)payload[22] * 16 * 16 + ((int)payload[21]))) / 10.0f;
                humidity = (((int)payload[24] * 16 * 16 + ((int)payload[23]))) / 10.0f;

                Thermometer thermometer = *Thermometer::get();
                auto callback = thermometer.getOnChange();
                if(callback != nullptr){
                    callback(temp, humidity);
                }
            }
        }
    }
};

Thermometer *Thermometer::thermometer = nullptr;

Thermometer::Thermometer()
{
    Serial.println("Begin setup BLE");

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99); // less or equal setInterval value
    Serial.println("End setup BLE");
}

Thermometer *Thermometer::get()
{
    if (Thermometer::thermometer == nullptr)
    {
        Thermometer::thermometer = new Thermometer();
    }
    return Thermometer::thermometer;
}

void Thermometer::loop()
{
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}

Thermometer::CallbackFunction Thermometer::getOnChange()
{
    return this->m_on_change;
}

void Thermometer::setOnChange(CallbackFunction callback)
{
    m_on_change = callback;
}
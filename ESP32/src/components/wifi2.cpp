/////////////////////////////////////////////////////////////////

#include "WiFi.h"
#include "wifi2.h"
#include "utils.h"
#include <Wire.h>
#include "EspMQTTClient.h"

/////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////

Wifi *Wifi::wifi = nullptr;
char buff[512];

//Wifi configuration
const char* ssid = "esp-4";
const char* password =  "Azeqsdwxc123";

//MQTT configuration
EspMQTTClient client(
  ssid,
  password,
  "192.168.136.232",  // MQTT Broker server ip
  "",   // Can be omitted if not needed
  "",   // Can be omitted if not needed
  "Nathan",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

Wifi::Wifi()
{
}

Wifi *Wifi::get()
{
    if (Wifi::wifi == nullptr)
    {
        Wifi::wifi = new Wifi();
    }
    return Wifi::wifi;
}

bool Wifi::verify_connection(){
    return client.isWifiConnected();
}

void Wifi::sendMqtt(string topic, float value){
    String topic2 = topic.c_str();
    String message = toString(value);
    client.publish(topic2, message);
}

void onConnectionEstablished(){}

void Wifi::loop(){
    client.loop();
}
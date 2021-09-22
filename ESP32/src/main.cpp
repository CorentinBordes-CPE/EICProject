/////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "components/thermometer.h"
#include "components/btn.h"
#include "components/wifi2.h"
#include "components/utils.h"
#include <thread>

/////////////////////////////////////////////////////////////////

using namespace std;

/////////////////////////////////////////////////////////////////

Btn btnRight(BUTTON_1);
Btn btnLeft(BUTTON_2);

Thermometer *thermometer;
std::thread *bluetoothThread;

Wifi *wifi;

float temp;
float humidity;

void setup()
{
	Serial.begin(115200);
	Serial.println("Start");

	//Set Wifi
	wifi = Wifi::get();

	//Set BLE
	thermometer = Thermometer::get();
	thermometer->setOnChange([](float t, float h){
		temp = t;
		humidity = h;
		Serial.printf("temp = %f, humidity = %f \n", temp, humidity);
		
		if(wifi->verify_connection()){
			wifi->sendMqtt("nathan/temp", temp);
			wifi->sendMqtt("nathan/humidity", humidity);
		}
	});

	bluetoothThread = new std::thread([]{
		while (true)
		{
			thermometer->loop();
		}
	});

	//Set buttons
	btnLeft.setClickHandler([](Btn &b){ 
		
	});
	btnRight.setClickHandler([](Btn &b){ 
		
	});
}

void button_loop()
{
	btnRight.loop();
	btnLeft.loop();
}

void loop()
{
	button_loop();
	wifi->loop();
}
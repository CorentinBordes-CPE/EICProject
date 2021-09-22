#include <Arduino.h>
#include "thermometer.h"
#include "btn.h"
#include <thread>

Btn btnRight(BUTTON_1);
Btn btnLeft(BUTTON_2); 

Thermometer *thermometer;
std::thread *bluetoothThread;

float temp;
float humidity;

void setup() {
  Serial.begin(115200);
  Serial.println("Start");

  //Set BLE
  thermometer = Thermometer::get();
  thermometer->setOnChange([](float t, float h){
    temp = t;
    humidity = h;
    Serial.printf("temp = %f, humidity = %f \n", temp, humidity);
  });

  bluetoothThread = new std::thread([]{
    while(true){
      thermometer->loop();
    }
  });

  //Set buttons
  btnLeft.setClickHandler([](Btn & b) {
    Serial.println("Left");
  });
  btnRight.setClickHandler([](Btn & b) {
    Serial.println("Right");
  });
}

void button_loop(){
  btnRight.loop();
  btnLeft.loop();
}

void loop() {
  button_loop();
}
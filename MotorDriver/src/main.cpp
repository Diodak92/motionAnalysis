#include <Arduino.h>
#include <arduino-timer.h>
#include <ArduinoJson.h>
#include <LoRa.h>
#include <AdaptiveValve.h>

// create a timer with default settings
auto timer = timer_create_default();
// Declare adaptive valve object
AdaptiveValve ad_valve_1(0b1001001, 3, 7);
AdaptiveValve ad_valve_2(0b1001001, 2, 6);

bool print_position(void *)
{
  ad_valve_1.print_position();
  return true;
}

void setup()
{
  // start serial comunication
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup started!");
  // check if ADC for vavle 1 was initalized successfully
  Serial.println(ad_valve_1.begin());
  Serial.println(ad_valve_2.begin());
  timer.every(500, print_position);
  Serial.println("Setup successfully completed!");
}

void loop()
{
  timer.tick();
  // open valve
  ad_valve_1.set_position(0);
  while (ad_valve_1.controller());
  Serial.println("Valve in position!");
  delay(2000);
  // close valve
  ad_valve_1.set_position(70);
  while (ad_valve_1.controller());
  Serial.println("Valve in position!");
  delay(2000);
}
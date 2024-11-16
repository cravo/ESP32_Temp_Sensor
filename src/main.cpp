#include <Arduino.h>

#include "temperature.h"
#include "web_server.h"
#include "wifi_connection.h"
#include "mqtt.h"

// update every 30 seconds
const uint32_t update_frequency_ms = 30 * 1000;

void setup(void) 
{
  Serial.begin(9600);
  wifi_connection_setup();
  temperature_setup();
  web_server_setup();  
  mqtt_setup();
}

void loop(void) 
{
  temperature_update();
  mqtt_loop();

  float temperature = temperature_get();
  mqtt_send_temperature(temperature);

  delay(update_frequency_ms);
}
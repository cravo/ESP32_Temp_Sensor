#include <Arduino.h>

#include "temperature.h"
#include "web_server.h"
#include "wifi_connection.h"

void setup(void) 
{
  Serial.begin(9600);
  wifi_connection_setup();
  temperature_setup();
  web_server_setup();
}

void loop(void) 
{
  temperature_update();
}
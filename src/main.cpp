#include <Arduino.h>

#include "temperature.h"
#include "web_server.h"
#include "wifi_connection.h"
#include "mqtt.h"
#include "display.h"
#include "display_diag.h"

// update every 30 seconds
const uint32_t update_frequency_ms = 30 * 1000;

void setup(void) 
{
  Serial.begin(9600);
  //displaydiag_setup();
  display_setup();
  wifi_connection_setup();
  temperature_setup();
  web_server_setup();  
  mqtt_setup();
}

void loop(void) 
{
  display_clear();

  //displaydiag_loop();
  temperature_update();
  mqtt_loop();

  float temperature = temperature_get();
  mqtt_send_temperature(temperature);

  char temptext[16];
  sprintf(temptext, "TEMP: %.1f", temperature);
  
  display_line(0);
  display_show(temptext);
  display_line(1);
  display_show(wifi_connection_ip());

  delay(update_frequency_ms);
}
#include <Arduino.h>

#include "temperature.h"
#include "web_server.h"
#include "wifi_connection.h"
#include "mqtt.h"
#include "display.h"
#include "display_diag.h"
#include "thingsboard.h"

// update every 30 seconds
const uint32_t update_frequency_ms = 30 * 1000;
uint32_t update_count = 0;

void setup(void) 
{
  Serial.begin(9600);
  //displaydiag_setup();
  display_setup();
  wifi_connection_setup();
  temperature_setup();
  web_server_setup();  
  //mqtt_setup();
  thingsboard_setup();
}

void loop(void) 
{
  thingsboard_update();

  if((update_count++) > update_frequency_ms)
  {
    display_clear();

    //displaydiag_loop();
    temperature_update();
    //mqtt_loop();

    float temperature = temperature_get();
    //mqtt_send_temperature(temperature);

    struct ThingsboardData thingsboardData;
    thingsboardData.temperature = temperature;
    thingsboard_send_data(thingsboardData);
    
    char temptext[16];
    sprintf(temptext, "TEMP: %.1f", temperature);
    
    display_line(0);
    display_show(temptext);
    display_line(1);
    display_show(wifi_connection_ip());

    update_count = 0;
  }

  delay(1);
}
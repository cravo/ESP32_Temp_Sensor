#ifndef __MQTT_H__
#define __MQTT_H__

void mqtt_setup();
void mqtt_loop();
void mqtt_send_temperature(float temperature);

#endif
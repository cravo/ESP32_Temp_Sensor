#include <SPI.h>
#include <PubSubClient.h>

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "secrets.h"



WiFiClient wifi_client;

PubSubClient mqtt_client(wifi_client);

void mqtt_reconnect()
{
  // Loop until we're reconnected
  while (!mqtt_client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(mqtt_client_name)) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish(mqtt_topic_log,"hello world");

    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_setup()
{
  mqtt_client.setServer(mqtt_broker, mqtt_port);
}

void mqtt_loop()
{
  if (!mqtt_client.connected()) 
  {
    mqtt_reconnect();
  }
  else
  {

  }

  mqtt_client.loop();
}

void mqtt_send_temperature(float temperature)
{
    String temperature_string("{ \"value\" : " + String(temperature) + " }");

    mqtt_client.publish(mqtt_topic_temperature, temperature_string.c_str());
}
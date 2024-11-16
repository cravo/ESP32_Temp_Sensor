#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "secrets.h"

void wifi_connection_setup()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED) 
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}
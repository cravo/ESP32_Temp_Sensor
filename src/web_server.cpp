//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include "temperature.h"

AsyncWebServer server(80);

const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "Not found");
}

void web_server_setup() 
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        String message;
        String temperature(temperature_get());

        message = "Current temp is " + temperature + " celcius";

        request->send(200, "text/plain", message);
    });

    server.onNotFound(notFound);

    server.begin();
}

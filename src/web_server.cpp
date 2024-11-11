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
        request->send(200, "text/plain", "Hello, world");
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) 
    {
        String message;
        String temperature(temperature_get());

        message = "Current temp is " + temperature + " celcius";

        // if (request->hasParam(PARAM_MESSAGE)) 
        // {
        //     message = request->getParam(PARAM_MESSAGE)->value();
        // } 
        // else 
        // {
        //     message = "No message sent";
        // }
        // request->send(200, "text/plain", "Hello, GET: " + message);

        request->send(200, "text/plain", message);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request)
    {
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) 
        {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } 
        else 
        {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });

    server.onNotFound(notFound);

    server.begin();
}

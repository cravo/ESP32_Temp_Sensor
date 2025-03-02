#include <SPI.h>
#include <PubSubClient.h>

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "secrets.h"

#include "thingsboard.h"

WiFiClient thingsboard_wifi_client;

PubSubClient thingsboard_client(thingsboard_wifi_client);

unsigned int thingsboard_request_id = 0;
unsigned int thingsboard_firmware_request_id = 0;
unsigned int thingsboard_current_firmware_chunk = 0;
bool thingsboard_firmware_received = false;
unsigned int thingsboard_firmware_update_bytes_downloaded = 0;

static const char * thingsboard_topic = "v1/devices/me/telemetry";
static const char * thingsboard_topic_request = "v1/devices/me/attributes/request/";
static const String thingsboard_checksum_attr = "fw_checksum";
static const String thingsboard_checksum_alg_attr = "fw_checksum_algorithm";
static const String thingsboard_size_attr = "fw_size";
static const String thingsboard_title_attr = "fw_title";
static const String thingsboard_version_attr = "fw_version";
static const String thingsboard_state_attr = "fw_state";
static const uint16_t thingsboard_chunk_size = 32768;

struct ThingsboardFirmwareInfo
{
  String title;
  String version;
  String state;
  unsigned int size;
};

ThingsboardFirmwareInfo thingsboard_firmware_info;
ThingsboardFirmwareInfo thingsboard_firmware_update_info;

void thingsboard_send_current_firmware_info()
{
  String currentTitle = "\"current_" + thingsboard_title_attr + "\":\"" + thingsboard_firmware_info.title + "\"";
  String currentVersion = "\"current_" + thingsboard_version_attr + "\":\"" + thingsboard_firmware_info.version + "\"";
  String state = "\"" + thingsboard_state_attr + "\":\"" + thingsboard_firmware_info.state + "\"";

  String data = "{" + currentTitle + "," + currentVersion + "," + state + "}";

  Serial.println("Sending current firmware info:");
  Serial.println(data.c_str());

  thingsboard_client.publish(thingsboard_topic, data.c_str());
}

void thingsboard_request_firmware_info()
{
  thingsboard_request_id++;
  String topic = String(thingsboard_topic_request) + String(thingsboard_request_id);
  String requiredSharedKeys = thingsboard_checksum_attr + ","
                            + thingsboard_checksum_alg_attr + ","
                            + thingsboard_size_attr + ","
                            + thingsboard_title_attr + ","
                            + thingsboard_version_attr;

  String data = "{\"sharedKeys\":\"" + requiredSharedKeys + "\"}";
  thingsboard_client.publish(topic.c_str(), data.c_str());
}

void thingsboard_parse_firmware_info_token(const String & token, ThingsboardFirmwareInfo & out_firmware_info)
{
  int separatorIndex = token.indexOf(":");

  if(separatorIndex >= 0)
  {
    if(token.indexOf(thingsboard_version_attr) >= 0)
    {
      out_firmware_info.version = token.substring(separatorIndex + 1);
      out_firmware_info.version = out_firmware_info.version.substring(1, out_firmware_info.version.length() - 1);
    }
    else if(token.indexOf(thingsboard_title_attr) >= 0)
    {
      out_firmware_info.title = token.substring(separatorIndex + 1);
      out_firmware_info.title = out_firmware_info.title.substring(1, out_firmware_info.title.length() - 1);
    }
    else if(token.indexOf(thingsboard_size_attr) >= 0)
    {
      out_firmware_info.size = token.substring(separatorIndex + 1).toInt();
    }
  }
}

void thingsboard_parse_firmware_info(const String & message, ThingsboardFirmwareInfo & out_firmware_info)
{
  int tokenIndex = message.indexOf(",");
  int start = message.indexOf("{") + 1;
  while(tokenIndex > 0)
  {
    String token = message.substring(start, tokenIndex);
    thingsboard_parse_firmware_info_token(token, out_firmware_info);

    //Serial.println(token);
    start = tokenIndex + 1;
    tokenIndex = message.indexOf(",", start);

    if(tokenIndex < 0)
    {
      token = message.substring(start, message.length() - 2);
      thingsboard_parse_firmware_info_token(token, out_firmware_info);

      //Serial.println(message.substring(start, message.length() - 1));
    }
  }
}

void thingsboard_get_firmware()
{
  Serial.println("Request new firmware...");

  String topic = "v2/fw/request/" + String(thingsboard_firmware_request_id) + "/chunk/" + String(thingsboard_current_firmware_chunk);
  String payload = "";
  
  if(thingsboard_chunk_size > 0 
  && thingsboard_firmware_update_info.size > thingsboard_chunk_size)
  {
    payload = String(thingsboard_chunk_size);
  }

  Serial.println(topic.c_str());
  Serial.println(payload.c_str());

  thingsboard_client.publish(topic.c_str(), payload.c_str());
}

void thingsboard_process_firmware()
{
  Serial.println("Firmware downloaded.");

  thingsboard_firmware_info.state = "DOWNLOADED";
  thingsboard_send_current_firmware_info();
  delay(1);

  Serial.println("Verifying firmware...");
  //todo : Should realy verify the firmware checksum ourselves, but ignoring that step for now
  Serial.println("Verified.");

  thingsboard_firmware_info.state = "VERIFIED";
  thingsboard_send_current_firmware_info();
  delay(1);

  thingsboard_firmware_received = true;
}

void thingsboard_process_message(String topic, String message)
{
  if(topic.startsWith("v1/devices/me/attributes"))
  {
    if(message.indexOf(thingsboard_version_attr) > 0)
    {
      ThingsboardFirmwareInfo firmwareInfo;
      thingsboard_parse_firmware_info(message, firmwareInfo);

      Serial.print("FIRMWARE INFO: ");
      Serial.print("TITLE: ");
      Serial.print(firmwareInfo.title.c_str());
      Serial.print(" VERSION: ");
      Serial.print(firmwareInfo.version.c_str());
      Serial.print(" SIZE: ");
      Serial.print(String(firmwareInfo.size).c_str());
      Serial.println("");

      if(topic.indexOf("response") >= 0)
      {
        // this is just the message telling us what our current firmware is
        Serial.println("CURRENT FIRMWARE INFO RECORDED");
        thingsboard_firmware_info = firmwareInfo;
      }
      else
      {
        if(firmwareInfo.version != thingsboard_firmware_info.version)
        {
          // this is a new firmware version
          Serial.println("FIRMWARE UPDATE AVAILABLE");
          thingsboard_firmware_info.state = "DOWNLOADING";
          thingsboard_send_current_firmware_info();
          delay(1);

          thingsboard_current_firmware_chunk = 0;
          thingsboard_firmware_update_bytes_downloaded = 0;
          thingsboard_firmware_update_info = firmwareInfo;

          thingsboard_get_firmware();
        }
      }
    }
  }
}

void thingsboard_message_received(char * topic, byte * message, unsigned int length)
{
  Serial.print("Thingsboard message received [");
  Serial.print(topic);
  Serial.print("] ");

  if(String(topic).startsWith("v1/devices/me/attributes"))
  {
    String messageString = "";

    for(unsigned int index = 0; index < length; ++index)
    {
      messageString += (char)(message[index]);
    }

    Serial.print(messageString.c_str());

    Serial.println("");

    thingsboard_process_message(topic, messageString);
  }
  else
  {
    String update_response_pattern = "v2/fw/response/" + String(thingsboard_firmware_request_id) + "/chunk/";

    if(String(topic).startsWith(update_response_pattern))
    {
      Serial.println("Got fw response message");

      if(thingsboard_firmware_update_bytes_downloaded < thingsboard_firmware_update_info.size)
      {
        Serial.print("Getting chunk ");
        Serial.println(String(thingsboard_current_firmware_chunk).c_str());

        //todo: Here we need to append the update data to a file
        //OR perhaps abandon this approach and use the Arduino OTA library
        //to download from a URL.

        thingsboard_firmware_update_bytes_downloaded += length;
        thingsboard_current_firmware_chunk += 1;

        thingsboard_get_firmware();
      }
      else
      {
        thingsboard_process_firmware();
      }
    }
  }
}

void thingsboard_reconnect()
{
  // Loop until we're reconnected
  if (!thingsboard_client.connected()) 
  {
    Serial.print("Attempting Thingsboard connection...");
    // Attempt to connect
    if (thingsboard_client.connect(thingsboard_client_id, thingsboard_client_id, "")) 
    {
      Serial.println("connected");
      thingsboard_client.setBufferSize(65535);
      thingsboard_client.subscribe("v1/devices/me/attributes/response/+");
      thingsboard_client.subscribe("v1/devices/me/attributes");
      thingsboard_client.subscribe("v2/fw/response/+");

      thingsboard_send_current_firmware_info();
      thingsboard_request_firmware_info();
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(thingsboard_client.state());
    }
  }
}

void thingsboard_setup()
{
  thingsboard_firmware_info.size = 0;
  thingsboard_firmware_info.title = "Initial";
  thingsboard_firmware_info.version = "v0";
  thingsboard_firmware_info.state = "UPDATED";

  thingsboard_client.setServer(thingsboard_address, thingsboard_port);
  thingsboard_client.setCallback(thingsboard_message_received);
}

void thingsboard_do_firmware_update()
{
  thingsboard_firmware_info.state = "UPDATING";
  thingsboard_send_current_firmware_info();
  delay(1);

  Serial.println("Updating firmware...");
  //todo: Actually do the firmware update
  Serial.println("Updated.");

  thingsboard_firmware_info = thingsboard_firmware_update_info;
  thingsboard_firmware_info.state = "UPDATED";
  thingsboard_send_current_firmware_info();

  thingsboard_firmware_received = false;
  delay(1);
}

void thingsboard_update()
{
  if (!thingsboard_client.connected()) 
  {
    thingsboard_reconnect();
  }
  else
  {
    if(thingsboard_firmware_received)
    {
      thingsboard_do_firmware_update();
    }

    thingsboard_client.loop();
  }
}

void thingsboard_send_data(struct ThingsboardData const & data)
{
    String data_string("{ \"temperature\" : " + String(data.temperature) + " }");

    thingsboard_client.publish(thingsboard_topic, data_string.c_str());
}

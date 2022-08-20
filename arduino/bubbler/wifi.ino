#ifdef ESP32
#ifdef ENABLE_WIFI

#include <WiFi.h>
#include <WiFiMulti.h>
#include <AsyncMqtt_Generic.h>

#include "arduino_secrets.h"

// Todo - let these be configureable some other way besides editing the source
// SSID and PW for your Router
const String Router_SSID = SECRET_SSID;
const String Router_Pass = SECRET_PASS;

// Your MQTT settings.
const String MQTT_HOST = SECRET_MQTT_HOST;
const int MQTT_PORT = SECRET_MQTT_PORT;
const String MQTT_USER = SECRET_MQTT_USER;
const String MQTT_PASS = SECRET_MQTT_PASS;
const String PubTopic = "bubbler/" + String(ESP.getEfuseMac(), HEX);

// Send WLED commands to light up LED with the same colors?
bool mqttSendWledCommands = true;
const String wledTopic = "wled/all/api";

WiFiMulti wimu;
AsyncMqttClient mqttClient;

void WiFiEvent(WiFiEvent_t event) {
  switch ( event ) {
    case ARDUINO_EVENT_WIFI_READY:
      Serial.println("WiFi is ready");
      break;
     case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("Connected to base station");
      break;
     case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
     case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("WiFi connection established, local IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("Connecting to MQTT...");
      mqttClient.connect();
      break;
     case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("WiFi lost IP");
      break;
     case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection to base");
  }
}

void onMqttConnect(bool sessionPresent)
{
  static char buf[8];
  Serial.print("Connected to MQTT broker: "); Serial.print(MQTT_HOST);
  Serial.print(", port: "); Serial.println(MQTT_PORT);
  Serial.print("PubTopic: "); Serial.println(PubTopic);
  mqttClient.publish((PubTopic + "/status").c_str(), 0, false, "connected");
  CRGB target = colorTarget;
  CRGB start = colorStart;

  snprintf(buf, 7, "%02X%02X%02X", start.r, start.g, start.b);
  mqttClient.publish((PubTopic + "/startcolor").c_str(), 0, true, buf);
  snprintf(buf, 7, "%02X%02X%02X", target.r, target.g, target.b);
  mqttClient.publish((PubTopic + "/targetcolor").c_str(), 0, true, buf);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void) reason;
  Serial.println("Disconnected from MQTT.");
}

void wifi_send_breath(bool b) {
  static char buf[100];
  String out = "in";
  if ( ! b )
    out = "done";
  mqttClient.publish((PubTopic + "/breath").c_str(), 0, false, out.c_str() );

  if ( ! mqttSendWledCommands )
    return;

  if ( b ) {
    CRGB start = colorStart;
    snprintf(buf, 100, "{\"bri\": 255, \"tt\": 0, \"seg\":[{\"fx\":0,\"col\":[[%d,%d,%d]]}]}", start.r, start.g, start.b);
    mqttClient.publish(wledTopic.c_str(), 0, false, buf);
    // skip the slow transition for the moment. It interferes with the fast one afterwards.
    //mqttClient.publish("wled/all/api", 0, false, "{\"bri\": 255, \"tt\": 64, \"seg\":[{\"col\":[[0,0,255]]}}" );
  } else {
    CRGB target = colorTarget;
    snprintf(buf, 100, "{\"bri\": 255, \"tt\": 16, \"seg\":[{\"fx\":0,\"col\":[[%d,%d,%d]]}]}", target.r, target.g, target.b);
    mqttClient.publish(wledTopic.c_str(), 0, false, buf);
  }
}

void wifi_setup() {
  Serial.println("Starting WiFi init");

  wimu.addAP(Router_SSID.c_str(), Router_Pass.c_str());
  Serial.printf("Trying to connect to %s with password %s\n", Router_SSID.c_str(), Router_Pass.c_str());

  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(MQTT_HOST.c_str(), MQTT_PORT);
  mqttClient.setCredentials(MQTT_USER.c_str(), MQTT_PASS.c_str());

  wimu.run();
}

#endif
#endif

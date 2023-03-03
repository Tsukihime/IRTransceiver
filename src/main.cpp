#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <PubSubClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <Ticker.h>

#include "mqtt.h"
#include "EEPROMSettings.h"
#include "AVEX_HVAC_IR.h"
#include "IRSender.h"
#include "Config.h"

const uint16_t RECV_PIN = 5;
const uint16_t IR_LED = 4;

IRrecv irrecv(RECV_PIN, 256);
IRsend irsend(IR_LED);

Ticker ticker;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

MQTT mqtt(mqttClient);
AVEX_HVAC_IR hvac(irsend, mqtt);
IRSender irsender(irsend, irrecv, mqtt);

void setup() {
    Serial.begin(115200);
    Serial.println();

    WiFi.hostname(Config::getDeviceName());

    EEPROMSettings settings;

    char *old_mqtt_server = settings.getMQTTserver();
    uint16_t old_mqtt_port = settings.getMQTTPort();

    WiFiManager wifiManager;
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", old_mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", String(old_mqtt_port).c_str(), 5);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.setTimeout(180);
    if (!wifiManager.autoConnect(Config::getDeviceName().c_str())) {
        Serial.println("RESTART!");
        ESP.restart();
    }

    const char *new_mqtt_server = custom_mqtt_server.getValue();
    uint16_t new_mqtt_port = String(custom_mqtt_port.getValue()).toInt();

    bool mqtt_server_changed = strcmp(new_mqtt_server, old_mqtt_server) != 0;
    bool mqtt_port_changed = new_mqtt_port != old_mqtt_port;

    if (mqtt_server_changed || mqtt_port_changed) {
        settings.setMQTTPort(new_mqtt_port);
        settings.setMQTTServer(new_mqtt_server);
        settings.save();
        Serial.println("MQTT settings updated");
    }

    mqtt.init(settings.getMQTTserver(), settings.getMQTTPort());

    ArduinoOTA.begin();
    irrecv.enableIRIn();
    irsend.begin();
    ticker.attach(60, []() {
        hvac.sendStateMQTT();
    });

//    digitalWrite(IR_LED, HIGH);
//    delay(7000);
//    digitalWrite(IR_LED, LOW);
}

void loop() {
    ArduinoOTA.handle();
    mqtt.loop();
    irsender.loop();
}

#include "mqtt.h"
#include "Config.h"

const String aircon_commands_prefix = "aircon/commands/";
const String aircon_state_prefix = "aircon/state/";
const String ir_send_topic = "ir/send";
const String ir_recv_topic = "ir/raw";

MQTT::MQTT(PubSubClient &client) : client(client) {
    port = 0;
}

void MQTT::init(const char *mqtt_domain, uint16_t mqtt_port) {
    domain = String(mqtt_domain);
    port = mqtt_port;
    client
            .setServer(domain.c_str(), mqtt_port)
            .setCallback(
                    [this](char *p_topic, byte *p_payload, unsigned int p_length) {
                        this->messageArrived(p_topic, p_payload, p_length);
                    });
}

void MQTT::subscribe() {
    String topic = Config::getMQTTTopicPrefix() + aircon_commands_prefix + "#";
    client.subscribe(topic.c_str());
    Serial.println("subscribe to topic: " + topic);

    topic = Config::getMQTTTopicPrefix() + ir_send_topic;
    client.subscribe(topic.c_str());
    Serial.println("subscribe to topic: " + topic);
}

void MQTT::setACCommandCallback(std::function<void(String &command, String &argument)> callback) {
    this->acCommandCallback = callback;
}

void MQTT::setIRCommandCallback(std::function<void(String &command)> callback) {
    this->irCommandCallback = callback;
}

void MQTT::messageArrived(char *p_topic, byte *p_payload, unsigned int p_length) {
    char c_payload[p_length + 1]; // +1 for null terminated string
    memcpy(c_payload, p_payload, p_length);
    c_payload[p_length] = 0; // null terminator

    String payload = String(c_payload);
    String topic = String(p_topic);

    Serial.println("messageArrived:");
    Serial.println("  " + topic + " -> " + payload);

    int cmd_offset = topic.indexOf(aircon_commands_prefix);
    int ir_offset = topic.indexOf(ir_send_topic);

    if (cmd_offset != -1) {
        String command = topic.substring(cmd_offset + aircon_commands_prefix.length());
        if (this->acCommandCallback != nullptr) {
            this->acCommandCallback(command, payload);
        }
    } else if (ir_offset != -1) {
        if (this->irCommandCallback != nullptr) {
            this->irCommandCallback(payload);
        }
    }
}

void MQTT::sendACState(const String &name, const String &value) {
    String topic = Config::getMQTTTopicPrefix() + aircon_state_prefix + name;
    client.publish(topic.c_str(), value.c_str());
}

void MQTT::sendIRRawData(const String &data) {
    String topic = Config::getMQTTTopicPrefix() + ir_recv_topic;
    client.publish(topic.c_str(), data.c_str());
}

void MQTT::sendMQTTDiscoveryConfig() {
    String cmd_topic = Config::getMQTTTopicPrefix() + aircon_commands_prefix;
    String state_topic = Config::getMQTTTopicPrefix() + aircon_state_prefix;
    String discovery_topic = "homeassistant/climate/avex_aircon/config";
    String device_object = R"({
        "identifiers": [")" + Config::getDeviceId() + R"("],
        "name": "Infrared Transceiver",
        "manufacturer": "Tsukihime",
        "model": "ESP8266-IRTransceiver",
        "sw_version": "0.9"
        })";

    String aircon_discovery_message = R"({
        "device": )" + device_object + R"(,
        "unique_id": "CH-AC07CHVITA-0417-P4-S0051",
        "icon": "mdi:air-conditioner",
        "name": "AVEX AC-07CH Vita",
        "modes": ["off", "auto", "cool", "heat", "dry", "fan_only"],
        "fan_modes": ["auto", "low", "medium", "high"],
        "swing_modes": ["on", "off"],
        "power_command_topic": ")" + cmd_topic + R"(power",
        "mode_command_topic": ")" + cmd_topic + R"(mode",
        "mode_state_topic": ")" + state_topic + R"(mode",
        "temperature_command_topic": ")" + cmd_topic + R"(temp",
        "temperature_state_topic": ")"   + state_topic + R"(temp",
        "fan_mode_command_topic": ")"      + cmd_topic + R"(fanspeed",
        "fan_mode_state_topic": ")"      + state_topic + R"(fanspeed",
        "swing_mode_command_topic": ")"    + cmd_topic + R"(swing",
        "swing_mode_state_topic": ")"    + state_topic + R"(swing",
        "current_temperature_topic": ")" + state_topic + R"(current_temp",
        "min_temp": 16, "max_temp": 32, "temp_step": 1, "retain": false
        })";
    client.publish(discovery_topic.c_str(), aircon_discovery_message.c_str(), true);
	
    discovery_topic = "homeassistant/sensor/irtransceiver_temp/config";
    String thermometer_discovery_message = R"({
        "device": )" + device_object + R"(,
        "unique_id": "irtransceiver_temp",
        "name": "Temperature",
        "state_topic": ")" + state_topic + R"(current_temp",
        "unit_of_measurement": "°C",
        "icon": "hass:thermometer",
        "value_template": "{{ value | round(1) }}"
        })";
    client.publish(discovery_topic.c_str(), thermometer_discovery_message.c_str(), true);
}

void MQTT::disconnect() {
    Serial.println("INFO: Closing the MQTT connection");
    client.disconnect();
}

void MQTT::reconnect() {
    while (!client.connected()) {
        Serial.println("INFO: Attempting MQTT connection...");
        if (client.connect(Config::getDeviceName().c_str(), "", "")) {
            Serial.println("INFO: connected");
            subscribe();
            sendMQTTDiscoveryConfig();
        } else {
            Serial.print("ERROR: failed, rc=");
            Serial.print(client.state());
            Serial.println("DEBUG: try again in 3 seconds");
            delay(3000);
        }
    }
}

void MQTT::loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

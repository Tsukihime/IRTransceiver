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

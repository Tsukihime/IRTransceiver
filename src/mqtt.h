#ifndef __MQTT_H
#define __MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>

class MQTT {
private:
    PubSubClient &client;
    String domain;
    uint16_t port;
    std::function<void(String &command, String &argument)> acCommandCallback;
    std::function<void(String &command)> irCommandCallback;

    void reconnect();
    void subscribe();
    void messageArrived(char *p_topic, byte *p_payload, unsigned int p_length);

public:
    explicit MQTT(PubSubClient &client);
    void init(const char *mqtt_domain, uint16_t mqtt_port);
    void loop();
    void sendIRRawData(const String &data);
    void sendACState(const String &name, const String &value);
    void disconnect();
    void setACCommandCallback(std::function<void(String &command, String &argument)> callback);
    void setIRCommandCallback(std::function<void(String &command)> callback);
};

#endif

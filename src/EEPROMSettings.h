#ifndef ESP8266_THERMOMETER_EEPROMSETTINGS_H
#define ESP8266_THERMOMETER_EEPROMSETTINGS_H

#include <PubSubClient.h>

class EEPROMSettings {
private:
    struct EEPROM_SETTINGS {
        char mqtt_server[40];
        uint16_t mqtt_port;
    } _settings;

public:
    EEPROMSettings();

    void save();

    char *getMQTTserver();

    void setMQTTServer(const char *server);

    uint16_t getMQTTPort();

    void setMQTTPort(uint16_t port);

};

#endif //ESP8266_THERMOMETER_EEPROMSETTINGS_H

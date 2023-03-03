#include "EEPROMSettings.h"
#include <EEPROM.h>

EEPROMSettings::EEPROMSettings() {
    EEPROM.begin(sizeof(_settings));
    EEPROM.get(0, _settings);
    EEPROM.end();
}

void EEPROMSettings::save() {
    EEPROM.begin(sizeof(_settings));
    EEPROM.put(0, _settings);
    EEPROM.commit();
    EEPROM.end();
}

char *EEPROMSettings::getMQTTserver() {
    return &_settings.mqtt_server[0];
}

void EEPROMSettings::setMQTTServer(const char *server) {
    strcpy(_settings.mqtt_server, server);
}

uint16_t EEPROMSettings::getMQTTPort() {
    return _settings.mqtt_port;
}

void EEPROMSettings::setMQTTPort(uint16_t port) {
    _settings.mqtt_port = port;
}
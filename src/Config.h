#ifndef CONFIG_H
#define CONFIG_H

#include "WString.h"
#include "Esp.h"

namespace Config {
    static const String DEVICE_NAME = "IRTransceiver";
    static const String MQTT_PARENT_TOPIC = "home";

    static String getDeviceName() {
        return DEVICE_NAME;// + "_" +  String(ESP.getChipId(), HEX);
    }

    static String getMQTTTopicPrefix() {
        return MQTT_PARENT_TOPIC + "/" + getDeviceName() + "/";
    }
};


#endif //CONFIG_H

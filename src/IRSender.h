//
// Created by VovaN on 08.09.2019.
//

#ifndef IRTRANSCEIVER_SOFT_IRSENDER_H
#define IRTRANSCEIVER_SOFT_IRSENDER_H


#include <IRrecv.h>
#include <IRsend.h>
#include "mqtt.h"

class IRSender {
private:
    IRsend &irsend;
    IRrecv &irrecv;
    MQTT &mqtt;

    void onIRCmdReceived(String &command);
public:
    IRSender(IRsend &irsend, IRrecv &irrecv, MQTT &mqtt);

    void loop();
};


#endif //IRTRANSCEIVER_SOFT_IRSENDER_H

#ifndef IRTRANSCEIVER_SOFT_AVEX_HVAC_IR_H
#define IRTRANSCEIVER_SOFT_AVEX_HVAC_IR_H
/*
A        B        C        D        E        F
...HHHHH ....tH.L ........ .FFA..PS MMMTTTTT 11010101

F = 0xd5

E = ...TTTTT     => T - число 0-16 температура 16-32
D = .FF.....     => F - число 0-3  скорость 0-auto, 1-fan3, 2-fan2, 3-fan1
E = MMM.....     => M - число 0-4 mode  auto-0, cool-1, dry-2, heat-4, fan-3
D = .......S     => sleep on-1, off-0
B = .......L     => lamp on-0, off-1
B = ....t...     => turbo on-1, off-0
B = .....H..     => hold on-1, off-0
D = ...A....     => air flow on-0, off-1
D = ......P.     => power on-1, off-0
D = ..SS....     => swing  SS: 10 => ON, 01 => OFF
A = ...HHHHH     => timer 0-off, 1-24 - hours
*/

#include <IRrecv.h>
#include <IRsend.h>
#include "mqtt.h"

const uint16_t TimeLag = 5;
const uint16_t HdrMark = 6110 - TimeLag;
const uint16_t HdrSpace = 7400 - TimeLag;
const uint16_t OneMark = 550 - TimeLag;
const uint16_t OneSpace = 580 - TimeLag;
const uint16_t ZeroMark = 550 - TimeLag;
const uint16_t ZeroSpace = 1650 - TimeLag;
const uint16_t EndMark = 550 - TimeLag;
const uint16_t EndSpace = 7420 - TimeLag;


namespace HVAC_Mode {         // MMM.....
    const uint8_t Automatic = {0b00000000};
    const uint8_t Cool      = {0b00100000};
    const uint8_t Dry       = {0b01000000};
    const uint8_t Fan_only  = {0b01100000};
    const uint8_t Heat      = {0b10000000};
}

namespace HVAC_FAN_Mode {     // .FF.....
    const uint8_t Automatic = {0b00000000};
    const uint8_t Low       = {0b00100000};
    const uint8_t Medium    = {0b01000000};
    const uint8_t High      = {0b01100000};
}

namespace HVAC_Swing {  // ..SS....
    const uint8_t On  = {0b00100000};
    const uint8_t Off = {0b00010000};
}

namespace HVAC_Power {  // ......P.
    const uint8_t On  = {0b00000010};
    const uint8_t Off = {0b00000000};
}

class AVEX_HVAC_IR {
private:
    boolean swing;
    boolean power;
    uint8_t h_mode;
    uint8_t fan_mode;
    uint8_t temperature;

    IRsend &irsend;
    MQTT &mqtt;

    void processPowerCmd(String &argument);

    void processModeCmd(String &argument);

    void processTempCmd(String &argument);

    void processSwingCmd(String &argument);

    void processFanspeedCmd(String &argument);

public:
    AVEX_HVAC_IR(IRsend &irsend, MQTT &mqtt);

    void setPower(boolean isOn);

    boolean getPower();

    void setMode(uint8_t mode);

    uint8_t getMode();

    void setFanMode(uint8_t fan_mode);

    uint8_t getFanMode();

    void setSwing(boolean isOn);

    boolean getSwing();

    void setTemperature(uint8_t temperature);

    uint8_t getTemperature();

    void processACCommand(String &command, String &argument);

    void updateState();

    void sendStateIR();

    void sendStateMQTT();
};


#endif //IRTRANSCEIVER_SOFT_AVEX_HVAC_IR_H

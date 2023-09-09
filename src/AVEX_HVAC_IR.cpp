#include "AVEX_HVAC_IR.h"
#include "mqtt.h"

AVEX_HVAC_IR::AVEX_HVAC_IR(IRsend &irsend, MQTT &mqtt) : irsend(irsend), mqtt(mqtt) {
    swing = true;
    power = false;
    h_mode = HVAC_Mode::Automatic;
    fan_mode = HVAC_FAN_Mode::Automatic;
    temperature = 25;
    current_temp = 0;
    mqtt.setACCommandCallback([this](String &command, String &argument) {
        this->processACCommand(command, argument);
    });
}

void AVEX_HVAC_IR::setMode(uint8_t mode) {
    h_mode = mode;
    updateState();
}

uint8_t AVEX_HVAC_IR::getMode() {
    return h_mode;
}

void AVEX_HVAC_IR::setFanMode(uint8_t mode) {
    fan_mode = mode;
    updateState();
}

uint8_t AVEX_HVAC_IR::getFanMode() {
    return fan_mode;
}

void AVEX_HVAC_IR::setSwing(boolean isOn) {
    swing = isOn;
    updateState();
}

boolean AVEX_HVAC_IR::getSwing() {
    return swing;
}

void AVEX_HVAC_IR::setPower(boolean isOn) {
    power = isOn;
    updateState();
}

boolean AVEX_HVAC_IR::getPower() {
    return power;
}

void AVEX_HVAC_IR::setTemperature(uint8_t temp) {
    if (temp < 16) { temp = 16; }
    if (temp > 32) { temp = 32; }
    temperature = temp;
    updateState();
}

uint8_t AVEX_HVAC_IR::getTemperature() {
    return temperature;
}

void AVEX_HVAC_IR::updateState() {
    sendStateIR();
    sendStateMQTT();
}

void AVEX_HVAC_IR::sendStateIR() {
    auto pow = power ? HVAC_Power::On : HVAC_Power::Off;
    auto temp = (temperature - 16u) & 0b000111111u;
    auto sw = swing ? HVAC_Swing::On : HVAC_Swing::Off;

    const uint8_t message_size = 12;
    uint8_t message_bytes[message_size];

    message_bytes[0] = 0x00;
    message_bytes[2] = 0x00;
    message_bytes[4] = 0x40;
    message_bytes[6] = static_cast<uint8_t>(sw | fan_mode) | pow; // uint8_t | uint8_t = int - sad but true
    message_bytes[8] = h_mode | temp;
    message_bytes[10] = 0xd5;

    for (int i = 0; i < message_size; i += 2) { // odd byte = inverted even
        message_bytes[i + 1] = ~message_bytes[i];
    }

    irsend.sendGeneric(HdrMark, HdrSpace,
                       OneMark, OneSpace,
                       ZeroMark, ZeroSpace,
                       OneMark, 0,
                       message_bytes, message_size,
                       38, false,
                       0, 50
    );
    irsend.space(EndSpace);
    irsend.mark(EndMark);
}

String modeToStr(uint8_t mode) {
    switch (mode) {
        case HVAC_Mode::Automatic :
            return "auto";
        case HVAC_Mode::Cool :
            return "cool";
        case HVAC_Mode::Heat:
            return "heat";
        case HVAC_Mode::Dry :
            return "dry";
        case HVAC_Mode::Fan_only :
            return "fan_only";
        default:
            return "off";
    }
}

String fanModeToStr(uint8_t mode) {
    switch (mode) {
        case HVAC_FAN_Mode::Medium :
            return "medium";
        case HVAC_FAN_Mode::Low:
            return "low";
        case HVAC_FAN_Mode::High :
            return "high";
        case HVAC_FAN_Mode::Automatic :
        default:
            return "auto";
    }
}

void AVEX_HVAC_IR::setCurrentTemp(float temperature) {
    current_temp = temperature;
}

float AVEX_HVAC_IR::getCurrentTemp() {
    return current_temp;
}

void AVEX_HVAC_IR::sendStateMQTT() {
    String mode = power ? modeToStr(h_mode) : "off";
    mqtt.sendACState("mode", mode);

    String temp = String(temperature);
    mqtt.sendACState("temp", temp);

    String fanspeed = fanModeToStr(fan_mode);
    mqtt.sendACState("fanspeed", fanspeed);

    String s_swing = swing ? "on" : "off";
    mqtt.sendACState("swing", s_swing);

    String s_temp = String(getCurrentTemp(), 1);
    mqtt.sendACState("current_temp", s_temp);
}

void AVEX_HVAC_IR::processPowerCmd(String &argument) {
    if (argument.equals("OFF")) {
        setPower(false);
    } else if (argument.equals("ON")) {
        setPower(true);
    }
}

void AVEX_HVAC_IR::processModeCmd(String &argument) {
    power = true;
    if (argument.equals("off")) {
        setPower(false);
    } else if (argument.equals("auto")) {
        setMode(HVAC_Mode::Automatic);
    } else if (argument.equals("cool")) {
        setMode(HVAC_Mode::Cool);
    } else if (argument.equals("heat")) {
        setMode(HVAC_Mode::Heat);
    } else if (argument.equals("dry")) {
        setMode(HVAC_Mode::Dry);
    } else if (argument.equals("fan_only")) {
        setMode(HVAC_Mode::Fan_only);
    }
}

void AVEX_HVAC_IR::processTempCmd(String &argument) {
    uint8_t temp = argument.toInt();
    setTemperature(temp);
}

void AVEX_HVAC_IR::processSwingCmd(String &argument) {
    setSwing(argument.equals("on"));
}

void AVEX_HVAC_IR::processFanspeedCmd(String &argument) {
    if (argument.equals("auto")) {
        setFanMode(HVAC_FAN_Mode::Automatic);
    } else if (argument.equals("low")) {
        setFanMode(HVAC_FAN_Mode::Low);
    } else if (argument.equals("medium")) {
        setFanMode(HVAC_FAN_Mode::Medium);
    } else if (argument.equals("high")) {
        setFanMode(HVAC_FAN_Mode::High);
    }
}

void AVEX_HVAC_IR::processACCommand(String &command, String &argument) {
    if (command.equals("power")) {
        // processPowerCmd(argument);
    } else if (command.equals("mode")) {
        processModeCmd(argument);
    } else if (command.equals("temp")) {
        processTempCmd(argument);
    } else if (command.equals("fanspeed")) {
        processFanspeedCmd(argument);
    } else if (command.equals("swing")) {
        processSwingCmd(argument);
    }
}

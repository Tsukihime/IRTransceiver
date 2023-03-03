#include "IRSender.h"
#include <IRutils.h>

IRSender::IRSender(IRsend &irsend, IRrecv &irrecv, MQTT &mqtt) : irsend(irsend), irrecv(irrecv), mqtt(mqtt) {
    mqtt.setIRCommandCallback([this](String &command) { this->onIRCmdReceived(command); });
}

//  freq: pulse, pause, pulse, p...
//  freq may be omitted, default 38kHz be used
void IRSender::onIRCmdReceived(String &command) {
    auto buffsize = irrecv.getBufSize();
    uint16_t raw[buffsize];
    uint16_t frequency = 38;
    uint16_t size = 0;

    int left, right;
    right = command.indexOf(':');
    if (right != -1) {
        frequency = command.substring(0, right).toInt();
        left = ++right;
    } else {
        left = 0;
        right = 0;
    }

    auto length = static_cast<int> (command.length());
    for (; right < length; right++) {
        auto ch = command.charAt(right);
        if (right == length - 1 || ch == ',') {
            raw[size] = command.substring(left, right).toInt();
            size++;
            left = right + 1;
        }
    }

    Serial.println("IR parsed ");
    Serial.print("     size: ");
    Serial.println(size);
    Serial.print("     data: [");
    for (unsigned int i = 0; i < size; i++) {
        Serial.print(raw[i]);
        if (i + 1 != size) {
            Serial.print(", ");
        }

    }
    Serial.println("]");
    Serial.print("frequency: ");
    Serial.println(frequency);

    irsend.sendRaw(raw, size, frequency);
}

String resultToMQTTString(const decode_results *const results) {
    String output = "";
    // Reserve some space for the string to reduce heap fragmentation.
    output.reserve(1536);  // 1.5KB should cover most cases.
    // Dump bytes
    for (uint16_t i = 1; i < results->rawlen; i++) {
        uint32_t usecs;

        for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX;
             usecs -= UINT16_MAX) {
            output += uint64ToString(UINT16_MAX);
            output += F(", 0, ");
        }

        output += uint64ToString(usecs, 10);
        if (i < results->rawlen - 1)
            output += F(", ");            // ',' not needed on the last one
    }
    return output;
}

void IRSender::loop() {
    decode_results results = {};

    if (irrecv.decode(&results)) {
        String output = resultToMQTTString(&results);
        mqtt.sendIRRawData(output);
        irrecv.resume(); // Receive the next value

        Serial.println("IR received:");
        Serial.print("  Size: ");
        Serial.println(results.rawlen);
        Serial.println("  Raw: " + output);
    }
}
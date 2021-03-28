
#include <stdint.h>
#include "modulInfo.h"
#include "Arduino.h"

CModulInfo ava;

constexpr const char CModulInfo::device_name[] = "AVA3";

const int8_t pinWORK = 26;
const int8_t pin10uA = 36;
const int8_t pin100mA = 37;
const int8_t pinCELL = 33;

typedef struct {
    AVA_RELAY relay;
    int8_t pinNum;
}RELAY_TO_PIN_RELATION;

RELAY_TO_PIN_RELATION getRelayToPin(AVA_RELAY relay) {
    RELAY_TO_PIN_RELATION relayToPinMap;
    relayToPinMap.relay = relay;
    switch (relay) {
        case AVA_RELAY::WORK:
            relayToPinMap.pinNum = pinWORK;
        break;
        case AVA_RELAY::CELL:
            relayToPinMap.pinNum = pinCELL;
        break;
        case AVA_RELAY::CURRENT_10uA:
            relayToPinMap.pinNum = pin10uA;
        break;
        case AVA_RELAY::CURRENT_100mA:
            relayToPinMap.pinNum = pin100mA;
        break;
        default:
            relayToPinMap.relay = AVA_RELAY::INCORRECT_RELAY;
            relayToPinMap.pinNum = -1;       
        break;
    }
    return relayToPinMap;
}


bool CModulInfo::initRelaysPins() {
    pinMode(pinWORK, OUTPUT);
    pinMode(pin10uA, OUTPUT);
    pinMode(pin100mA, OUTPUT);
    pinMode(pinCELL, OUTPUT);    
    return true;
}


bool CModulInfo::init() {
    // here place init code for ava3
    add_console_command("system", this);
    initRelaysPins();
    return true;
}


void CModulInfo::relaySwitch(AVA_RELAY relay, int8_t val) {
    RELAY_TO_PIN_RELATION relayToPin = getRelayToPin(relay);
    if (relayToPin.relay != AVA_RELAY::INCORRECT_RELAY) {
        digitalWrite(relayToPin.pinNum, val ? HIGH : LOW);
    }
}

int8_t CModulInfo::readRelayPin(AVA_RELAY relay) {
    RELAY_TO_PIN_RELATION relayToPin = getRelayToPin(relay);
    int8_t res = 0;
    if (relayToPin.relay != AVA_RELAY::INCORRECT_RELAY) {
        res = digitalRead(relayToPin.pinNum);
    }
    return res;
}

void CModulInfo::relayTurnOn(AVA_RELAY relay) {
    relaySwitch(relay, 1);
}

void CModulInfo::relayTurnOff(AVA_RELAY relay) {
    relaySwitch(relay, 0);
}

void CModulInfo::relayToggle(AVA_RELAY relay) {
    int8_t val = readRelayPin(relay);
    val ^= 1;
    relaySwitch(relay, val);
}

void CModulInfo::Command(int argc, char* argv[]) {
    if (argc >= 2) {
        my_printf("receive system command\r\n");
        if (!strcmp("?", argv[1])) {
            my_printf("Output help info\r\n");
        }
    }
}

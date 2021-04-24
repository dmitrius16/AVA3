
#include <stdint.h>
#include "modulInfo.h"
#include "Drivers/subsytemDAC_ADC.h"
#include "Arduino.h"
#include "esp_ipc.h"

CModulInfo ava;

constexpr const char CModulInfo::device_name[] = "AVA3";

const int8_t pinWORK = 4;  //26
const int8_t pin10uA =  22;    //36;
const int8_t pin100mA = 23; //37;
const int8_t pinCELL = 21;//33;

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

bool CModulInfo::initSpiSubsystem() {
    
}


bool CModulInfo::init() {
    // here place init code for ava3
    add_console_command("sys", this);
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

void CModulInfo::relayToggle(AVA_RELAY relay, bool consOut) {
    int8_t val = readRelayPin(relay);
    if (consOut) {
        my_printf("Current state relay %s\r\n", val ? "on" : "off");
    }
    val ^= 1;
    relaySwitch(relay, val);
    if (consOut) {
        my_printf("Set state relay to %s\r\n", val ? "on" : "off");
    }
}

void CModulInfo::Command(int argc, char* argv[]) {
    
    if (argc >= 2) {
        //my_printf("receive system command\r\n");
        if (!strcmp("?", argv[1])) {
            my_printf("adc_start - start 100 adc asking\r\n");
            my_printf("Relay commands:\r\n");
            my_printf("\trelay_name = {work, cell, curr10uA, curr100mA} {\"\", on, off}\r\n");
            my_printf("\tif command after relay name is empty toggle state is occured\r\n");
        } else if (!strcmp("adc_start", argv[1])) {
            //test code execute on core 0 uncomment if nedded
            //esp_err_t ret = esp_ipc_call(0, init_analog_subsystem, nullptr); //delegate task to core 0
            //if (ret == ESP_OK) {
            //    ret = esp_ipc_call(0, start_adc_asking, nullptr);
            //}
            init_analog_subsystem(nullptr);
            start_adc_asking(nullptr);


            //test code 
            
            
            
            
            /* working code if it's needed uncoment
            CAnalogSubsystem& analogSubsystem = getAnalogSubsystemInstance();
            if (analogSubsystem.isInit() == false) {
                bool res = analogSubsystem.init();
                my_printf("Init analog subsystem %s\r\n", res ? "Ok" : "Err");
            }*/
        } else {    //here relay command
            typedef struct {
                const char *pName;
                AVA_RELAY relay;
            }relay_to_name_map;
            relay_to_name_map relayNames[] = {{"work", AVA_RELAY::WORK}, 
                                            {"cell", AVA_RELAY::CELL}, 
                                            {"curr10uA", AVA_RELAY::CURRENT_10uA}, 
                                            {"curr100mA", AVA_RELAY::CURRENT_100mA}};
            for (int i = 0; i < sizeof(relayNames) / sizeof(relayNames[0]); i++) {
                if (!strcmp(relayNames[i].pName, argv[1])) {
                    if (argc > 2) { // execute command 
                        if (!strcmp("on", argv[2])) {
                            relayTurnOn(relayNames[i].relay);
                            my_printf("turn on relay\r\n");
                        } else if (!strcmp("off", argv[2])) {
                            relayTurnOff(relayNames[i].relay);
                            my_printf("turn off relay\r\n");
                        } else {
                            my_printf("undefined commamd\r\n");
                        }
                    } else { // make toggle
                        relayToggle(relayNames[i].relay, true);
                    }
                }
            }
        }
        
        
        
    }
}

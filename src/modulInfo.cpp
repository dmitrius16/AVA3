
#include <stdint.h>
#include "modulInfo.h"
#include "Drivers/subsytemDAC_ADC.h"
#include "driver/mcpwm.h"
#include "Arduino.h"
#include "esp_ipc.h"

CModulInfo ava;

constexpr const char CModulInfo::device_name[] = "AVA3";

const int8_t pinWORK = 4;
const int8_t pin10uA =  22;
const int8_t pin100mA = 23;
const int8_t pinCELL = 21;

const int8_t pinMOT_PWM   = 25;     // PWM signal - output
const int8_t pinMOT_TACHO = 34;     // input pin for read analog value by ADC
const int8_t pinMOT_GATE  = 26;     // TACHO switch - output
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
    return true; 
}


bool CModulInfo::init() {
    // here place init code for ava3
    add_console_command("sys", this);
    initRelaysPins();
    initMotorDriver();
    return true;
}

bool CModulInfo::initMotorDriver() {
    // init gpio pins 
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, pinMOT_PWM);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, pinMOT_GATE);

    //config pwm parameters 
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 200; //200Hz
    pwm_config.cmpr_a = 50; // dirty cycle PWMA = 50 %
    pwm_config.cmpr_b = 50; // dirty cycle PWMB = 40 %
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_MODE, 5000, 5000);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
    
    
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
            my_printf("Motor commands:\r\n");
            my_printf("\tmotor_gate {\"\", on, off}\r\n");
            //my_printf("")
            //----------------------- Relay commands block
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

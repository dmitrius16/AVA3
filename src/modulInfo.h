#pragma once
#include "Applications/console.h"

#define DEVICE_NAME "AVA3"
// OS configuration
constexpr int MAX_COUNT_MUTEX = 20; 
// end OS configuration

enum class AVA_RELAY {
    WORK,
    CELL,
    CURRENT_10uA,
    CURRENT_100mA,
    INCORRECT_RELAY
};
class CModulInfo : public ConsoleCmd {
public:
    static const char device_name[]; 
private:
    bool initRelaysPins();
    bool initSpiSubsystem();
    void relaySwitch(AVA_RELAY relay, int8_t val);
    int8_t readRelayPin(AVA_RELAY relay);
    bool initMotorDriver();
public:
    bool init();
    void relayTurnOn(AVA_RELAY relay);
    void relayTurnOff(AVA_RELAY relay);
    void relayToggle(AVA_RELAY relay, bool consOut = false);
public: //console interface
    virtual void Command(int argc, char* argv[]);

};

extern CModulInfo ava;


#pragma once
#include <stdint.h>
class CAnalogSubsystem;

CAnalogSubsystem& getAnalogSubsystemInstance(void);
class CAnalogSubsystem {
friend CAnalogSubsystem& getAnalogSubsystemInstance(void);
private:
    bool m_bInit;
private:
    CAnalogSubsystem();
    bool initPereodicADCAsking();
    bool initSpiMasterSubsystem();
    bool initSpiSlaveSubsystem();
    bool initBUSYinput();
    bool initDAC_AD5761();
    bool sendCommandToDAC(uint32_t cmd);
public:
    bool init();
    bool isInit() {return m_bInit;}
    
};

void init_analog_subsystem(void* arg);
void start_adc_asking(void* arg); 

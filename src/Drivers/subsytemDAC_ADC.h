#pragma once

class CAnalogSubsystem;

CAnalogSubsystem& getAnalogSubsystemInstance(void);
class CAnalogSubsystem {
friend CAnalogSubsystem& getAnalogSubsystemInstance(void);
private:
    bool m_bInit;
private:
    CAnalogSubsystem();
    bool initPereodicADCAsking();
    bool initSpiSubsystem();
    bool initBUSYinput();
    bool initDAC_AD5761();
public:
    bool init();
    bool isInit() {return m_bInit;}
    
};

void init_analog_subsystem(void* arg);
void start_adc_asking(void* arg); 

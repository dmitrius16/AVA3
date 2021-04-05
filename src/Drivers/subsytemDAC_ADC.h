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
public:
    bool init();
    bool isInit() {return m_bInit;}
    
};



#pragma once

class CAVA3StateMachine {
private:
    bool m_bWaitingCmd;
    bool m_bSendingDataPkt;
    bool m_bExperimentOn;
    bool m_bSendingResponse;
    bool m_bCyclesPhase;
    bool m_bDebugMode;
//experiment parameters:  ### may be declare this variables in separate class

    uint8_t n1; 
    uint8_t n2;
    uint8_t n3;
    uint8_t n_count;
    uint32_t m_cntExpTermination;
//
    Stream *pLink;
private:
    void terminateExp() {
        n1 = n2 = n3 = n_count = 0;
        m_bSendingDataPkt = false;
        m_bExperimentOn = false;
        m_bSendingResponse = false;
        m_cntExpTermination++;
    }
public:
    CAVA3StateMachine(): m_bWaitingCmd(true), m_bSendingDataPkt(false), m_bExperimentOn(false), m_bSendingRespond(false), m_bCyclesPhase(false),
    m_bDebugMode(false), n1(0),n2(0), n3(0), n_count(0), m_cntExpTermination(0) {

    }
    bool init();
    bool isWaitingCmd() {return m_bWaitingCmd;} 
    void processLink();

};




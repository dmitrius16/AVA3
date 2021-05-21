#pragma once
#include <stdint.h>
#include "runnable_task.h"
#include "AVA3Protocol.h"
class Stream;
class CAVA3StateMachine : public RunnableTask{
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
// end experiment parameters

// counters.... and other staff
    uint8_t m_DataPktSendBytes;
    uint16_t m_GetInfoParam;
// 
    AVA3Commands m_curCmd;
    
    Stream *pLink;
private:
    void terminateExp() {
        n1 = n2 = n3 = n_count = 0;
        m_bSendingDataPkt = false;
        m_bExperimentOn = false;
        m_bSendingResponse = false;
        m_cntExpTermination++;
    }
    void outputCmdParamDbgFunc(int param_length);
    void processWaitingCmdState();
    void processRxCmdParamState();
    void parseInitParam(); 
    void parseGetAsParam() {}
    void parseSetParam();
    void parseGetInfo() {}
    void parseSetDataPktIdx() {}
    bool isDebugMode() {return m_bDebugMode;}
    void errRxCmdParameters(int waitLen, int RxLen);
public:
    CAVA3StateMachine(): m_bWaitingCmd(true), m_bSendingDataPkt(false), m_bExperimentOn(false), m_bSendingResponse(false), m_bCyclesPhase(false),
    m_bDebugMode(true), n1(0),n2(0), n3(0), n_count(0), m_cntExpTermination(0), m_DataPktSendBytes(0), m_GetInfoParam(0),
    m_curCmd(AVA3Commands::Cmd_undefined_cmd) {

    }
    bool init();
    bool isWaitingCmd() {return m_bWaitingCmd;} 
    void processLink();
    //virtual functions from RunnableTask
    virtual bool run_task();
    virtual bool on_init_process(void *param=nullptr);
};

extern CAVA3StateMachine g_AVA3StateMachine;


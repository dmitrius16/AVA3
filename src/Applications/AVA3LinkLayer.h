#pragma once
#include <stdint.h>
#include "runnable_task.h"
#include "AVA3Protocol.h"
#include "ExperimentParam.h"
class Stream;
class CExpParam;

class CAVA3LinkLayer : public RunnableTask{
private:
    bool m_bWaitingCmd;
    bool m_bSendingResponse;
    bool m_bSendingDataPkt; 
    
    bool m_bDebugMode;

    uint8_t m_SendByteIdx;
    uint8_t m_RespSendByteIdx;
// counters.... and other staff
    uint8_t m_DataPktSendBytes;
    uint16_t m_GetInfoParam;
// 
// do_send_packets 
    uint16_t m_DataSendBytes;
    uint16_t m_DataSizeBytes;
    uint16_t m_DataPtr;
//
    AVA3Commands m_curCmd;
    
    Stream *m_pLink;
    CExpParam *m_pExpParam; // we use local copy in LinkLayer! if state change we send it to AVA3StateMachine 
private:
    /*
    void terminateExp() {
        n1 = n2 = n3 = n_count = 0;
        m_bSendingDataPkt = false;
        m_bExperimentOn = false;
        m_bSendingResponse = false;
        m_cntExpTermination++;
    }*/
    void outputCmdParamDbgFunc(int param_length);
    void processWaitingCmdState();
    void processRxCmdParamState();
    void parseInitParam(); 
    void parseGetAsParam(); 
    void parseSetParam();
    void parseGetInfo();
    void parseSetDataPktIdx();
    bool isDebugMode() {return m_bDebugMode;}
    void errRxCmdParameters(int waitLen, int RxLen);
    void setDataSendingState();
    void do_send_pkts();
    uint8_t makeStatusByte();
public:
    CAVA3LinkLayer(): m_bWaitingCmd(true), m_bSendingResponse(false), m_bSendingDataPkt(false), 
    m_bDebugMode(true), m_SendByteIdx(0), m_RespSendByteIdx(0), m_DataPktSendBytes(0), m_GetInfoParam(0),
    m_curCmd(AVA3Commands::Cmd_undefined_cmd) {

    }
    bool init();
    bool isWaitingCmd() {return m_bWaitingCmd;} 
    void processLink();
    //virtual functions from RunnableTask
    virtual bool run_task();
    virtual bool on_init_process(void *param=nullptr);
};

extern CAVA3LinkLayer g_AVA3Link;


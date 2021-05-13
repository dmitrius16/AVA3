#include "AVA3.h"
#include "Arduino.h"
#include "console.h"
#include "MyLib/OSWrappers.h"

CAVA3StateMachine g_AVA3StateMachine;
static uint8_t rx_buffer[40];


//constants
const uint16_t IP_EXP_ON = 1;


const char* GetNameCommand(uint8_t code) {
    static const char *pCmdNames[] = {"CMD_CALC_DATA_PKT_COUNT", "CMD_START_EXP", "CMD_QUERY_STATE",
                                    "CMD_QUERY_STATUS_AS", "CMD_GET_DATA_PKT", "CMD_INIT", "CMD_GET_INFO",
                                    "CMD_REP_GET_DATA_PKT", "CMD_SET_DATA_PKT_IDX", "CMD_SET_PARAMS", 
                                    "CMD_GET_AS_PARAM", "CMD_TERMINATE_EXP"};
    int ind = -1;
    const char *res = "Undefined cmd";
    if (code >= 0xf0 && code <= 0xf8) {
        ind = code - 0xf0;
    } else if (code == 0xfa) {
        ind = 9;
    } else if (code == 0xfb) {
        ind = 10;
    } else if (code == 0xff) {
        ind = 11;
    }

    if (ind != -1) {
        res = pCmdNames[ind];        
    } 
    return res; 
}

const char* GetAnswerNames(uint8_t code) {
   
   static const char *pAnswerNms[] = {"ANS_CALC_DATA_PKT_COUNT", "ANS_START_EXP", "ANS_QUERY_STATE",
                                    "ANS_TERMINATE_EXP", "ANS_INIT", "ANS_REP_GET_DATA_PKT", "ANS_SET_DATA_PKT_IDX",
                                    "ANS_SET_PARAMS", "ANS_GET_AS_PARAM" };
    uint8_t code_buff[] = {0x1, 0x2, 0x3, 0xA, 0xf5, 0xf7, 0xf8, 0xfa, 0xfb}; 
    int i = 0;
    for (; i < sizeof(code_buff)/sizeof(code_buff[0]); i++) {
        if (code == code_buff[i]) {
            break;
        }
    }
    return i < sizeof(code_buff)/sizeof(code_buff[0]) ? pAnswerNms[i] : "Undefined Answer";
}



bool CAVA3StateMachine::init() {
    pLink = &Serial; 
    //
    // Add here some init code for AVA state machine
    //

    return true;
}
void CAVA3StateMachine::processLink() {
    if (isWaitingCmd()) {    
        processWaitingCmdState();
    } else {
        processRxCmdParamState();
    }
}

void CAVA3StateMachine::processWaitingCmdState() {
    if (pLink->available()) {
        uint8_t read_byte = pLink->read();
        AVA3Commands rxCmd = (AVA3Commands)read_byte;
        switch (rxCmd) {
        case AVA3Commands::Cmd_terminate_exp: 
            terminateExp();
            pLink->write((uint8_t)AVA3Answers::Ans_terminate_exp);
            break;
        case AVA3Commands::Cmd_calc_data_pkt_count:
            // 
            // command hasn't implemented yet!!!
            // SET_DATA_SENFING_STATE
            // DataPtr = StartDataPtr
            pLink->write((uint8_t)AVA3Answers::Ans_calc_data_pkt_count);
            break;
        case AVA3Commands::Cmd_start_exp:
            // command hasn't implemented yet!!!
            //ps = RAM_BASE_PTR
            m_bExperimentOn = true;
            //ExpSetupOffset = RAM_BASE
            //RAMPtr = ncount_w = nw = 0;
            pLink->write((uint8_t)AVA3Answers::Ans_start_exp);
            break;
        case AVA3Commands::Cmd_query_state: //implement first in queue!!!
            // command hasn't implemented yet
            // SendByteIdx = RespSendByteIdx = 0;
            m_bSendingResponse = true;
            pLink->write((uint8_t)AVA3Answers::Ans_query_state);
            break;
        case AVA3Commands::Cmd_query_status_as:
            //sample_ok - variable in ofirginal always set to false
            pLink->write((uint8_t)0);
            break;
        case AVA3Commands::Cmd_get_data_pkt:
            m_bSendingDataPkt = true;
            m_DataPktSendBytes = 0;
            //operation hasn't implemented yet
            //DataPtr -= DATA_PACKET_SIZE
            break;
        case AVA3Commands::Cmd_set_data_pkt_idx:
        case AVA3Commands::Cmd_get_info:
            m_bWaitingCmd = false;
            m_curCmd = rxCmd;
            break;
        case AVA3Commands::Cmd_init:
        case AVA3Commands::Cmd_set_params:
        case AVA3Commands::Cmd_get_as_param:
            if (!m_bExperimentOn) {
                m_bWaitingCmd = false;
                m_curCmd = rxCmd;
            }
            break;
        } 
    }
}

void CAVA3StateMachine::processRxCmdParamState() {
    int rxBytes = 0;
    switch(m_curCmd) {
        case AVA3Commands::Cmd_init:
            rxBytes = pLink->readBytes(rx_buffer, 9);
            if (rxBytes == 9) {
                parseInitParam();
                m_bWaitingCmd = true;
                pLink->write((uint8_t)AVA3Answers::Ans_init);
            } // else {                    
                // error occur in rx parameters
            //}
        break;
        case AVA3Commands::Cmd_get_as_param:
            rxBytes = pLink->readBytes(rx_buffer, 4);
            if (rxBytes == 4) {
                parseGetAsParam();
                m_bExperimentOn = m_bWaitingCmd = true;
                //sample_ok = false;
                pLink->write((uint8_t)AVA3Answers::Ans_get_as_param); 
            } // else {
                // error occur in rx parameters
            //}
        break;
        case AVA3Commands::Cmd_set_params:
            //ps = RAM_BASE_PTR;
            rxBytes = pLink->readBytes(rx_buffer, 4);
            if (rxBytes == 4) {                    
               parseSetParam();
               m_bWaitingCmd = true;
               pLink->write((uint8_t)AVA3Answers::Ans_set_params); 
            } // else {
                // error occur in rx parameters
            //}
        break;
        case AVA3Commands::Cmd_get_info:
            rxBytes = pLink->readBytes(rx_buffer, 2);
            if (rxBytes == 2) {
                parseGetInfo();
                uint8_t txVal = 0;
                if (m_GetInfoParam == IP_EXP_ON) {
                    txVal = m_bExperimentOn ? 1 : 0;
                }
                pLink->write(txVal);
                m_bWaitingCmd = true;
            } // else {
                // error occur in rx parameters
            // }
        break;
        case AVA3Commands::Cmd_set_data_pkt_idx:
            rxBytes = pLink->readBytes(rx_buffer, 2);
            if (rxBytes == 2) {
                parseSetDataPktIdx();
                //DataSentBytes = DATA_PACKET_SIZE * PktIndex;
                //                DataPtr = START_DATA_PTR + DataSentBytes;
                //                RespSendByteIdx = 0;
                //                SET_DATA_SENDING_STATE;
                m_bWaitingCmd = true;
                pLink->write((uint8_t)AVA3Answers::Ans_set_data_pkt_idx);
            }
        break;
    }
}

bool CAVA3StateMachine::on_init_process(void *param) {
    init();
    return true;
}

bool CAVA3StateMachine::run_task() {
    uint8_t rxBuf[80];
    while(true) {
        taskDelayMs(1);
        int numRXBytes =  pLink->available();
        if (numRXBytes) {
            numRXBytes = pLink->readBytes(rxBuf, 80);
            // send echo to console
            for (int i = 0; i < numRXBytes; i++) {
                my_printf("0x%X ", rxBuf[i]);
            }
            my_printf("\r\n");
        }
    }
    return true;
}

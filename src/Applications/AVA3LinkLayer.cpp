#include "AVA3LinkLayer.h"
#include "ExperimentParam.h"
#include "Arduino.h"
#include "console.h"
#include "MyLib/OSWrappers.h"

CAVA3LinkLayer g_AVA3Link;
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



bool CAVA3LinkLayer::init() {
    pLink = &Serial; 
    m_pExpParam = getExpParamInstance(); 
    return true;
}
void CAVA3LinkLayer::processLink() {
    if (isWaitingCmd()) {    
        processWaitingCmdState();
    } else {
        processRxCmdParamState();
    }
}

void CAVA3LinkLayer::processWaitingCmdState() {
    if (pLink->available()) {
        uint8_t read_byte = pLink->read();
        AVA3Commands rxCmd = (AVA3Commands)read_byte;
        bool answerAvaliable = false;
        uint8_t answer_code = 0;

        if (isDebugMode()) {
            const char* pCmdName = GetNameCommand(read_byte);
            my_printf("Rx command - %s code: 0x%X\r\n", pCmdName, read_byte);
        }

        switch (rxCmd) {
        case AVA3Commands::Cmd_terminate_exp: 
            m_pExpParam->terminateExp();
            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_terminate_exp; 

            break;
        case AVA3Commands::Cmd_calc_data_pkt_count:
            // 
            // command hasn't implemented yet!!!
            // SET_DATA_SENFING_STATE
            // DataPtr = StartDataPtr

            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_calc_data_pkt_count;

            break;
        case AVA3Commands::Cmd_start_exp:
            // command hasn't implemented yet!!!
            //ps = RAM_BASE_PTR
            
            //need method for set parameters simultaneously
            m_pExpParam->SetExperimentOn(true); // m_bExperimentOn = true;
            m_pExpParam->set_phase_cycles(0);
            

            //ExpSetupOffset = RAM_BASE
            //RAMPtr = ncount_w = nw = 0;


            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_start_exp;



            break;
        case AVA3Commands::Cmd_query_state: //implement first in queue!!!
            // command hasn't implemented yet
            // SendByteIdx = RespSendByteIdx = 0;
            m_bSendingResponse = true;
            
            
            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_query_state;

            break;
        case AVA3Commands::Cmd_query_status_as:
            //sample_ok - variable in orirginal always set to false
            answerAvaliable = true;
            answer_code = 0;
            
            
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
            if (m_pExpParam->isExperimentOff()) {
                m_bWaitingCmd = false;
                m_curCmd = rxCmd;
            }
            break;
        default:
            if (isDebugMode()) {
                my_printf("Unknown command received\r\n");
            }
            break;
        } 
        if (answerAvaliable) {
            pLink->write(answer_code);
            if (isDebugMode()) {
                my_printf("Answer code: %d\r\n", answer_code);
            }
        }
    }
}

void CAVA3LinkLayer::errRxCmdParameters(int waitLen, int RxLen) {
    
    m_bWaitingCmd = true;
    if (isDebugMode())
        my_printf("Error rx cmd parameters. Wait %d bytes, received %d\r\n", waitLen, RxLen);    
}


void CAVA3LinkLayer::outputCmdParamDbgFunc(int param_length) {
    char temp[80] = {0};
    for (int i = 0; i < param_length; i++) {
        sprintf(temp, "%s 0x%X",temp, rx_buffer[i]);
    }
    strcat(temp, "\r\n");
    my_printf(temp);
}


void CAVA3LinkLayer::parseInitParam() {
    if (isDebugMode()) {
        outputCmdParamDbgFunc(g_length_init_param);
    }

    m_pExpParam->SetInitParam(rx_buffer, g_length_init_param);
}

void CAVA3LinkLayer::parseGetAsParam() {
    //
    //sample_i, nw - not used in original code. So this func empty
    //
}

void CAVA3LinkLayer::parseSetParam() {
    if (isDebugMode()) {
        outputCmdParamDbgFunc(g_length_set_params);
    }

    m_pExpParam->SetParams(rx_buffer, g_length_set_params);
}

void CAVA3LinkLayer::processRxCmdParamState() {
    int rxBytes = 0;
    int wait_length_rx = 0;
    switch (m_curCmd) {
        case AVA3Commands::Cmd_init:                    //implement, need to test!
            wait_length_rx = g_length_init_param;
            rxBytes = pLink->readBytes(rx_buffer, wait_length_rx);
            if (rxBytes == wait_length_rx) {
                parseInitParam(); 
                m_bWaitingCmd = true;
                pLink->write((uint8_t)AVA3Answers::Ans_init);
            }  else {                    
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_get_as_param:    //implement, need to test!

            wait_length_rx = 4;
            rxBytes = pLink->readBytes(rx_buffer, wait_length_rx);
            
            if (rxBytes == wait_length_rx) {
                parseGetAsParam();
                m_bWaitingCmd = true;
                m_pExpParam->SetExperimentOn(true);
                
                //sample_ok = false;
                pLink->write((uint8_t)AVA3Answers::Ans_get_as_param); 
            }  else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_set_params:       //implement, need to test!

            wait_length_rx = g_length_set_params; 
            rxBytes = pLink->readBytes(rx_buffer, wait_length_rx);
            
            if (rxBytes == wait_length_rx) {                    
               parseSetParam();
               m_bWaitingCmd = true;
               pLink->write((uint8_t)AVA3Answers::Ans_set_params); 
            } else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_get_info:    //implement, need to test!

            wait_length_rx = 2; 
            rxBytes = pLink->readBytes(rx_buffer, wait_length_rx);
            if (rxBytes == wait_length_rx) {
                parseGetInfo();
                uint8_t txVal = 0;
                if (m_GetInfoParam == IP_EXP_ON) {
                   txVal = m_pExpParam->isExperimentOn() ? 1 : 0;
                }
                pLink->write(txVal);
                m_bWaitingCmd = true;
            } else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_set_data_pkt_idx:  // not implemented yet
            wait_length_rx = 2;
            rxBytes = pLink->readBytes(rx_buffer, wait_length_rx);
            if (rxBytes == wait_length_rx) {
                parseSetDataPktIdx();
                //DataSentBytes = DATA_PACKET_SIZE * PktIndex;
                //                DataPtr = START_DATA_PTR + DataSentBytes;
                //                RespSendByteIdx = 0;
                //                SET_DATA_SENDING_STATE;
                m_bWaitingCmd = true;
                pLink->write((uint8_t)AVA3Answers::Ans_set_data_pkt_idx);
            } else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        default:
        
        break;
    }
}

bool CAVA3LinkLayer::on_init_process(void *param) {
    init();
    return true;
}

bool CAVA3LinkLayer::run_task() {
    //uint8_t rxBuf[80];
    while(true) {
        taskDelayMs(1); 
        processLink();
    }
    return true;
}


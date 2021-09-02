#include "AVA3LinkLayer.h"
#include "Arduino.h"
#include "console.h"
#include "MyLib/OSWrappers.h"

CAVA3LinkLayer g_AVA3Link;
static uint8_t rx_buffer[40];
static uint8_t tx_buffer[40];

//now it's fivtive variables
uint8_t g_voltageHiB, g_voltageLoB;
uint8_t g_currentHiB, g_currentLoB;
//now it's fictive variables


//constants
const uint16_t IP_EXP_ON = 1;
const uint16_t EXP_MEAS_SIZE = 4;
const uint16_t START_DATA_PTR = EXP_MEAS_SIZE;
const uint16_t DATA_PACKET_SIZE = 60;

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
    m_pLink = &Serial; 
    m_pExpParam = getExpParamInstance(); 
    return true;
}

void CAVA3LinkLayer::setDataSendingState() {
    m_SendByteIdx = 7; // do_send_pkts work in sending data mode
}

void CAVA3LinkLayer::do_send_pkts() {
    if (m_bSendingDataPkt || m_bSendingResponse) {
        if (m_SendByteIdx == 0) { //prepare and send first block data (name in enum SEND_STATUS)
            tx_buffer[0] = makeStatusByte();            
            tx_buffer[1] = m_pExpParam->get_n1_cnt();
            tx_buffer[2] = m_pExpParam->get_n1_cnt();
            tx_buffer[3] = m_pExpParam->get_n3_cnt();
            uint16_t amountCycles = m_pExpParam->get_amount_cycles();
            tx_buffer[4] = (uint8_t)(amountCycles >> 8);
            tx_buffer[5] = (uint8_t)(amountCycles);
            m_pLink->write(tx_buffer, 6);
            m_SendByteIdx = 7;           // it's similar to emum data type
        } else if (m_SendByteIdx == 7) {    //(SEND_DATA)
            if (m_bSendingResponse) {
                
                tx_buffer[0] = g_voltageHiB;    //fictive data
                tx_buffer[1] = g_voltageLoB;
                tx_buffer[2] = g_currentHiB;
                tx_buffer[3] = g_currentLoB;
                m_pLink->write(tx_buffer, 4);

                m_bSendingResponse = false;
            } else { // here we read memory and send data



            }
        }
    }
}

uint8_t CAVA3LinkLayer::makeStatusByte() {
    uint8_t res = 0;
    uint8_t temp = m_pExpParam->isCyclicPhase() ? 1 : 0;
    res |= temp << 6;
    temp = m_bSendingResponse ? 1 : 0;
    res |= temp  << 5;
    temp = m_bSendingDataPkt ? 1 : 0;
    res |= temp << 4;
    temp = m_bWaitingCmd ? 1 : 0;
    res |= res << 1;
    temp = m_pExpParam->isExperimentOn() ? 1 : 0;
    res |= temp;
    return res;
}

void CAVA3LinkLayer::processLink() {
    if (isWaitingCmd()) {    
        processWaitingCmdState();
    } else {
        processRxCmdParamState();
    }

    //do send packets

}

void CAVA3LinkLayer::processWaitingCmdState() {
    if (m_pLink->available()) {
        uint8_t read_byte = m_pLink->read();
        AVA3Commands rxCmd = (AVA3Commands)read_byte;
        bool answerAvaliable = false;
        uint8_t answer_code = 0;

        if (isDebugMode()) {
            const char* pCmdName = GetNameCommand(read_byte);
            my_printf("Rx command - %s code: 0x%X\r\n", pCmdName, read_byte);
        }

        switch (rxCmd) {
        case AVA3Commands::Cmd_terminate_exp: 
            m_pExpParam->terminateExpCmd();
            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_terminate_exp; 

            break;
        case AVA3Commands::Cmd_calc_data_pkt_count:
            m_SendByteIdx = m_DataSendBytes = 0;
            setDataSendingState();
            m_DataPtr = START_DATA_PTR;
            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_calc_data_pkt_count;

            break;
        case AVA3Commands::Cmd_start_exp: 
            m_pExpParam->experimentOnCmd(); 
            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_start_exp;
            break;
        case AVA3Commands::Cmd_query_state: // ready to use
            m_SendByteIdx = m_RespSendByteIdx = 0;
            m_bSendingResponse = true; 
            answerAvaliable = true;
            answer_code = (uint8_t)AVA3Answers::Ans_query_state;

            break;
        case AVA3Commands::Cmd_query_status_as:
            //sample_ok - variable in orirginal always set to false
            answerAvaliable = true;
            answer_code = 0; 
            break;
        case AVA3Commands::Cmd_get_data_pkt:    //+ 
            m_bSendingDataPkt = true;
            m_DataPktSendBytes = 0;
            break;
        case AVA3Commands::Cmd_rep_get_data_pkt: //+
            m_bSendingDataPkt = true;
            m_DataPktSendBytes = 0;
            m_DataPtr -= DATA_PACKET_SIZE;
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
            m_pLink->write(answer_code);
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

void CAVA3LinkLayer::parseSetDataPktIdx() {
    if (isDebugMode()) {
        outputCmdParamDbgFunc(g_length_set_pktIDX_param);
    }
    uint16_t pktInd = rx_buffer[0] << 8;
    pktInd |= rx_buffer[1];
    m_DataSendBytes = DATA_PACKET_SIZE * pktInd;
    m_DataPtr = START_DATA_PTR + m_DataSendBytes;
    m_RespSendByteIdx = 0;
    setDataSendingState();
}

void CAVA3LinkLayer::parseGetInfo() {
    if (isDebugMode()) {
        outputCmdParamDbgFunc(g_length_get_info_param);
    }

    m_GetInfoParam = rx_buffer[0] << 8;
    m_GetInfoParam |= rx_buffer[1];
}

void CAVA3LinkLayer::processRxCmdParamState() {
    int rxBytes = 0;
    int wait_length_rx = 0;
    switch (m_curCmd) {
        case AVA3Commands::Cmd_init:                    //implement, need to test!
            wait_length_rx = g_length_init_param;
            rxBytes = m_pLink->readBytes(rx_buffer, wait_length_rx);
            
            if (rxBytes == wait_length_rx) {
                parseInitParam(); 
                m_bWaitingCmd = true;
                m_pLink->write((uint8_t)AVA3Answers::Ans_init);
            }  else {                    
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_get_as_param:    //implement, need to test!

            wait_length_rx = 4;
            rxBytes = m_pLink->readBytes(rx_buffer, wait_length_rx);
            
            if (rxBytes == wait_length_rx) {
                parseGetAsParam();
                m_bWaitingCmd = true;
                m_pExpParam->SetExperimentOn(true);
                
                //sample_ok = false;
                m_pLink->write((uint8_t)AVA3Answers::Ans_get_as_param); 
            }  else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_set_params:       //implement, need to test!

            wait_length_rx = g_length_set_params; 
            rxBytes = m_pLink->readBytes(rx_buffer, wait_length_rx);
            
            if (rxBytes == wait_length_rx) {                    
               parseSetParam();
               m_bWaitingCmd = true;
               m_pLink->write((uint8_t)AVA3Answers::Ans_set_params); 
            } else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_get_info:    //implement, need to test!

            wait_length_rx = 2; 
            rxBytes = m_pLink->readBytes(rx_buffer, wait_length_rx);
            if (rxBytes == wait_length_rx) {
                parseGetInfo();
                uint8_t txVal = 0;
                if (m_GetInfoParam == IP_EXP_ON) {
                   txVal = m_pExpParam->isExperimentOn() ? 1 : 0;
                }
                m_pLink->write(txVal);
                m_bWaitingCmd = true;
            } else {
                errRxCmdParameters(wait_length_rx, rxBytes);
            }
        break;
        case AVA3Commands::Cmd_set_data_pkt_idx:  // not implemented yet
            wait_length_rx = 2;
            rxBytes = m_pLink->readBytes(rx_buffer, wait_length_rx);
            if (rxBytes == wait_length_rx) {
                parseSetDataPktIdx();
                m_bWaitingCmd = true;
                m_pLink->write((uint8_t)AVA3Answers::Ans_set_data_pkt_idx);
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


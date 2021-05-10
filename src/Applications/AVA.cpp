#include "AVA3.h"
#include "Arduino.h"
#include <stdint.h>

enum class AVA3Commands {
    Cmd_calc_data_pkt_count = 0xf0, 
    Cmd_start_exp           = 0xf1,
    Cmd_query_state         = 0xf2,
    Cmd_query_status_as     = 0xf3,
    Cmd_get_data_pkt        = 0xf4,
    Cmd_init                = 0xf5,
    Cmd_get_info            = 0xf6,
    Cmd_rep_get_data_pkt    = 0xf7,
    Cmd_set_data_pkt_idx    = 0xf8,
    Cmd_set_params          = 0xfa,
    Cmd_get_as_param        = 0xfb,
    Cmd_terminate_exp       = 0xff,
};


enum class AVA3Answers {
    Ans_calc_data_pkt_count = 0x1,
    Ans_start_exp           = 0x2,
    Ans_query_state         = 0x3,
    Ans_terminate_exp       = 0xA,
    Ans_init                = 0xf5,
    Ans_rep_get_data_pkt    = 0xf7,
    Ans_set_data_pkt_idx    = 0xf8,
    Ans_set_params          = 0xfa,
    Ans_get_as_param        = 0xfb,
};

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
    int ind = -1;                                    
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
        while (pLink->available()) {
            uint8_t read_byte = pLink->read();
            AVA3Commands curCmd = (AVA3Commands)read_byte;
            switch (curCmd) {
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

            } 
        }
}

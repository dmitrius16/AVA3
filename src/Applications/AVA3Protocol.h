#pragma once
enum class AVA3Commands {
    Cmd_undefined_cmd       = 0x0,
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

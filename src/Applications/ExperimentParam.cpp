#include "ExperimentParam.h"

uint8_t g_MemoryBuf[32767]; // imitation fram on PIC

uint8_t g_ParamMemory[500]; //size 500 it's just for experiment

const int RAM_SHIFT = 31000;

uint8_t* g_RamBasePtr = &g_ParamMemory[RAM_BASE];


CExpParam* getExpParamInstance(void) {
    static CExpParam* s_pExpPar = nullptr;
    if (s_pExpPar == nullptr) {
        s_pExpPar = new CExpParam();
    }
    return s_pExpPar;
}

void CExpParam::SetParams(uint8_t* bufParam, int length) {
    if (length == g_length_set_params) {
        uint16_t ramInd =  bufParam[0] << 8;
        ramInd += bufParam[1];
        //ramInd = RAM_SHIFT - ramInd + RAM_BASE;  
        ramInd = RAM_BASE + ramInd - RAM_SHIFT;
        g_ParamMemory[ramInd] = bufParam[2];
        g_ParamMemory[ramInd + 1] = bufParam[3]; 
    }
}



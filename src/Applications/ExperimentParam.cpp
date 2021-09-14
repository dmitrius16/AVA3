#include "ExperimentParam.h"

constexpr int SizeParamMemory = 1024;

uint8_t g_MemoryBuf[32767]; // imitation fram on PIC

uint8_t g_ParamMemory[SizeParamMemory]; //size 500 it's just for experiment

const int RAM_SHIFT = 31000;

uint8_t* g_RamBasePtr = &g_ParamMemory[RAM_BASE];

bool checkParamMemoryAddr(uint16_t addr) {
    return addr < SizeParamMemory;
}

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

void CExpParam::writeToParamMemory(uint16_t addr, uint8_t val) {
    if (!checkParamMemoryAddr(addr)) {
        return;
    }
    g_MemoryBuf[addr] = val;
}

void CExpParam::loadUByte(uint16_t addr, uint8_t& val) {
   if (!checkParamMemoryAddr(addr)) {
       return;
   } 
   val = g_MemoryBuf[addr];
}

void CExpParam::loadUShort(uint16_t addr, uint16_t &val) {
    if(!checkParamMemoryAddr(addr + 1)) {   
        return;
    }
    val = g_MemoryBuf[addr] << 8;
    val |= g_MemoryBuf[addr + 1];
}

void CExpParam::loadUInt(uint16_t addr, uint32_t &val) {
    if(!checkParamMemoryAddr(addr + 2)) {
        return;
    }
        val = g_MemoryBuf[addr] << 16;
        val |= g_MemoryBuf[addr + 1] << 8;
        val |= g_MemoryBuf[addr + 2];
}




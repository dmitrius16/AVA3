#pragma once
#include <stdint.h>
#include "AVA3Protocol.h"

class CExpParam;
CExpParam* getExpParamInstance(void);

constexpr uint16_t INFINITE_CYCLES = 0xFFFF;

class CExpParam {
    friend CExpParam* getExpParamInstance(void); 

    bool m_bExperimentOn;
    bool m_bSendingDataPkt;
    bool m_bSendingResponse;
    bool m_bCyclePhase;

    uint8_t n1;
    uint8_t n2;
    uint8_t n3;
    uint8_t n1_cnt;
    uint8_t n2_cnt;
    uint8_t n3_cnt;
    uint16_t m_nCount;
    uint16_t m_CycleRamBase;
    uint16_t m_DataSizeBytes;
    uint32_t m_cntExpTermination;
private:
    CExpParam() : m_bExperimentOn(false), m_bSendingResponse(false), m_bCyclePhase(false),
    n1(0), n2(0), n3(0), n1_cnt(0), n2_cnt(0), n3_cnt(0), m_nCount(0), m_CycleRamBase(0), m_DataSizeBytes(0) ,m_cntExpTermination(0) {}
public:
    void terminateExp() {
        n1 = n2 = n3 = 0;
        m_nCount = 0;
        m_bSendingDataPkt = false;
        m_bExperimentOn = false;
        m_bSendingResponse = false;
        m_cntExpTermination++;
    }

    uint8_t get_n1() {return n1;}
    uint8_t get_n2() {return n2;}
    uint8_t get_n3() {return n3;}

///   this members change only in state_machine, link layer change it too but only zero 
    void set_phase_cycles(uint8_t val) {
        n1_cnt = n2_cnt = n3_cnt = val; // this analog n1w, n2w, n3w from ava.c
    }

    void set_n1_cnt(uint8_t val) {
        n1_cnt = val;
    }

    void set_n2_cnt(uint8_t val) {
        n2_cnt = val;
    }

    void set_n3_cnt(uint8_t val) {
        n3_cnt = val;
    }

    uint8_t get_n1_cnt() {return n1_cnt;}
    uint8_t get_n2_cnt() {return n2_cnt;}
    uint8_t get_n3_cnt() {return n3_cnt;}

    void incr_n1_cnt() {n1_cnt += 1;}
    void incr_n2_cnt() {n2_cnt += 1;}
    void incr_n3_cnt() {n3_cnt += 1;}
///
    

    uint16_t getCycles() {return m_nCount;} // or may be rename m_nCount
    bool isInfiniteCycles() {return m_nCount == INFINITE_CYCLES;}
    
    bool isExperimentOn() {
        //use sync object
        return m_bExperimentOn == true;
    }

    bool isExperimentOff() {
        return m_bExperimentOn == false;
    }

    void SetExperimentOn(bool val) {
        // use sync object
        m_bExperimentOn = val;
    }

    void SetInitParam(uint8_t *bufParam, int length) {
        if (length == g_length_init_param) {
            
            n1 = bufParam[0];
            n2 = bufParam[1];
            n3 = bufParam[2];

            m_nCount = (uint16_t)bufParam[3] << 8;
            m_nCount += bufParam[4];

            m_CycleRamBase = (uint16_t)bufParam[5] << 8;
            m_CycleRamBase += bufParam[6];

            m_DataSizeBytes = (uint16_t)bufParam[7] << 8;
            m_DataSizeBytes += bufParam[8];

        }
    }
    void SetParams(uint8_t* bufParam, int length);
};

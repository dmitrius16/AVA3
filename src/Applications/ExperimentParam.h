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
    uint16_t m_nCount;
    uint16_t m_CycleRamBase;
    uint16_t m_DataSizeBytes;
    uint32_t m_cntExpTermination;
private:
    CExpParam() : m_bExperimentOn(false), m_bSendingResponse(false), m_bCyclePhase(false),
    n1(0), n2(0), n3(0), m_nCount(0), m_CycleRamBase(0), m_DataSizeBytes(0) ,m_cntExpTermination(0) {}
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

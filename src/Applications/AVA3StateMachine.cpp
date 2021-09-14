#include "AVA3StateMachine.h"
#include "ExperimentParam.h"
#include "../MyLib/OSWrappers.h"
#include "esp_timer.h"
#include "console.h"

CAVA3StateMachine g_AVA3StateMachine;



bool CAVA3StateMachine::on_init_process(void *param) {
    m_pExpParam = getExpParamInstance();
    return true;
}

void CAVA3StateMachine::init() {
    //ps = RAM_BASE_PTR; undefined purpose
    //RAMOff = RAM_SHIFT;
    //FSR0 = 0x1ff;
    //ExpSetupOffset = 0x51;
    m_pExpParam->set_ExpSetupOffset(0x51);
    for (int16_t DAC_Step = 0; DAC_Step <= 0xcff; DAC_Step++) {
        m_pExpParam->writeToParamMemory(0x200, m_pExpParam->get_ExpSetupOffset());
    }
    //....
}

bool CAVA3StateMachine::run_task() {
    int64_t res = 0;

    while(true) {
        
      //  res = esp_timer_get_time();
        taskDelayMs(1);
      //  my_printf("Current time %lld\r\n", res);

        if (isExperimentOn()) {
            setCyclePhase(false);    
            
            m_pExpParam->set_n1_cnt(0);
            //for (n1_cnt = 0; n1_cnt < m_pExpParam->get_n1(); n1_cnt++) {  //get_n1 - need to implement
            for(;m_pExpParam->get_n1_cnt() < m_pExpParam->get_n1(); m_pExpParam->incr_n1_cnt()) {
                make_pulse();
                if (isExperimentOff()) {
                    break;
                }
            }  
//----------------------------------------------------------
            if (isInfiniteCycles()) {
                while(isExperimentOn()) {
                    make_cycle_phase();
                }
            } else {
                
                // think about wrappers on cycles wariable in AVA3StateMachine class
                m_pExpParam->set_cycle_num(0);
                
                for(; m_pExpParam->get_cycle_num() < m_pExpParam->get_amount_cycles(); m_pExpParam->incr_cycle_cnt()) {
                    make_cycle_phase();
                    if (isExperimentOff()) {
                        break;
                    }
                }

            }
//------------------------------------------------------------
            setCyclePhase(false);

            m_pExpParam->set_n3_cnt(0);
            //for (uint8_t n3_cnt = 0; n3_cnt < m_pExpParam->get_n3(); n3_cnt++) {
            for(;m_pExpParam->get_n3_cnt() < m_pExpParam->get_n3(); m_pExpParam->incr_n3_cnt()) {
                make_pulse();
                if (isExperimentOff()) {
                    break;
                }
            }

            m_pExpParam->SetExperimentOn(false);

        }
    }

    return true;
}

inline bool CAVA3StateMachine::isInfiniteCycles() {
    
    return m_pExpParam->isInfiniteCycles();

}

inline uint16_t CAVA3StateMachine::get_cycles() {
    
    return m_pExpParam->get_amount_cycles();

}

inline bool CAVA3StateMachine::isExperimentOn() {
    return m_pExpParam->isExperimentOn();
}

inline bool CAVA3StateMachine::isExperimentOff() {
    return !m_pExpParam->isExperimentOn();
}

void CAVA3StateMachine::make_pulse() {
    // load experiment setup parameters from memory
    uint16_t startOffset = m_pExpParam->get_ExpSetupOffset();
    startOffset++; //preinc 
    uint8_t ctrlByte = 0;
    m_pExpParam->loadUByte(startOffset, ctrlByte);
    startOffset++;
    uint16_t DAC_value = 0;
    m_pExpParam->loadUShort(startOffset, DAC_value);
    startOffset += 2;
    uint32_t DealayU = 0;
    m_pExpParam->loadUInt(startOffset, DealayU);
    startOffset += 3;   //we read 3 bytes from memory
    uint32_t DelayI = 0;
    m_pExpParam->loadUInt(startOffset, DelayI);
    startOffset += 3;
    uint16_t DACStep = 0;
    m_pExpParam->loadUShort(startOffset, DACStep);
    startOffset += 2;
    uint16_t DACStepCount = 0;
    m_pExpParam->loadUShort(startOffset, DACStepCount);

     /* ctrlByte bits: 
                0 - turn on microelectrode
                1 - turn on motor
                2 - potentiostat / galvanostats
                3 - work
                4 - electrode range 10 uA
                5 - 0 - comparsion electrode, 1 - auxiliary electrode
                6 - cell
                7 - electrode range 100 mA
        */

}

void CAVA3StateMachine::make_cycle_phase() {

}
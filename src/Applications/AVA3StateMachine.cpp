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
                for(uint16_t cycle_cnt = 0; cycle_cnt < getCycles(); cycle_cnt++) {
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

inline uint16_t CAVA3StateMachine::getCycles() {
    return m_pExpParam->getCycles();
}

inline bool CAVA3StateMachine::isExperimentOn() {
    return m_pExpParam->isExperimentOn();
}

inline bool CAVA3StateMachine::isExperimentOff() {
    return !m_pExpParam->isExperimentOn();
}

void CAVA3StateMachine::make_pulse() {
    // load experiment setup parameters from memory
    

}

void CAVA3StateMachine::make_cycle_phase() {

}
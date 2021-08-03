#pragma once
#include <stdint.h>
#include "runnable_task.h"
#include "ExperimentParam.h"

class CExpParam;
class CAVA3StateMachine : public RunnableTask {
   
    bool m_CyclicPhase;     //to exp parameters

    CExpParam *m_pExpParam;
private:
    //this function work through call wrapper object
    bool isExperimentOn(); 
    bool isExperimentOff();
    void setCyclePhase(bool val) {
        m_CyclicPhase = val;
    }
    
    bool isInfiniteCycles();
    
    uint16_t get_cycles();   //check return type
    // experiment parameter object

    //experiment action functions
    void make_pulse();
    void make_cycle_phase();
public:
    virtual bool run_task();
    virtual bool on_init_process(void *param=nullptr);

};

extern CAVA3StateMachine g_AVA3StateMachine;

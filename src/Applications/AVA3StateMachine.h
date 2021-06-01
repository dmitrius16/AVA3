#pragma once
#include "runnable_task.h"

class CAVA3StateMachine : public RunnableTask {
    volatile bool m_ExperimentOn;

public:
    virtual bool run_task();
    virtual bool on_init_process(void *param=nullptr);

    //logic function
    bool IsExperimentOn() {return m_ExperimentOn;}
};

extern CAVA3StateMachine g_AVA3StateMachine;

#pragma once
#include "runnable_task.h"
#include "console.h"

class CSerialConsole : public RunnableTask {
    CConsole *m_pConsTask;
    ConsoleTask m_taskFunction;
public:
    bool on_init_process(void *param = nullptr);
    bool run_task();
};

extern CSerialConsole g_SerialConsole;
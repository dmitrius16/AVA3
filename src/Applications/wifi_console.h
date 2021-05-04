#pragma once
#include "runnable_task.h"
#include "console.h"
class WiFiConsole : RunnableTask {
    int m_port;
    CConsole *m_pConsTask;
    ConsoleTask m_taskFunction;
public:
    WiFiConsole(int port = 23) : m_port(port) {}
    bool on_init_process(void *param = nullptr);
    bool run_task();
};

extern WiFiConsole g_wifiConsole;


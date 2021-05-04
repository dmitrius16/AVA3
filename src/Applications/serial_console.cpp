#include "serial_console.h"
#include "console.h"
#include "Arduino.h"

CSerialConsole g_SerialConsole;

bool CSerialConsole::on_init_process(void *param) {
    m_pConsTask = GetConsoleInstance();
    m_pConsTask->init();
    m_pConsTask->setStreamInterface(&Serial);
    m_taskFunction = &CConsole::processStream;
    return true;
}

bool CSerialConsole::run_task() {
    m_pConsTask->printPrompt();

    while(1) {

        taskDelayMs(1);
        (m_pConsTask->*m_taskFunction)();

    }
    return true;
}


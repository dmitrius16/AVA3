#pragma once
////#include "MyLib/OSWrappers.h"
class RunnableTask {
public:
    virtual bool run_task() = 0;
    virtual bool on_init_process(void* param = nullptr) = 0;
};



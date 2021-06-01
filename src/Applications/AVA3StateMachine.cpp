#include "AVA3StateMachine.h"
#include "../MyLib/OSWrappers.h"
#include "esp_timer.h"
#include "console.h"

CAVA3StateMachine g_AVA3StateMachine;



bool CAVA3StateMachine::on_init_process(void *param) {
    return true;
}

bool CAVA3StateMachine::run_task() {

    //try init library

// debug code !!!!
  

    int64_t res = 0;

    while(true) {
        res = esp_timer_get_time();
        taskDelayMs(1000);
         my_printf("Current time %lld\r\n", res);

       // my_printf("Hello from core 0 current time\r\n");
    }

    return true;
}
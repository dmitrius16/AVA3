#include <Arduino.h>
#include "modulInfo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/////#include "sdkconfig.h"  ////??????
#include "Applications/runnable_task.h"
#include "Applications/serial_console.h"
#include "Applications/wifi_console.h"

#define DEBUG_MODE
#define CONFIG_USE_WIFI_CONSOLE

void common_task(void *arg) {
    RunnableTask *pTask = reinterpret_cast<RunnableTask*>(arg);
    if(pTask->on_init_process()) {
        pTask->run_task();
    }
}


void setup() {
  // put your setup code here, to run once:
  
  // here we need create console application  
  Serial.begin(115200);
  ava.init();

#if defined(CONFIG_USE_WIFI_CONSOLE)
    Serial.println("Start wifi console\r\n");
  //  xTaskCreatePinnedToCore(common_task, "Console", 4096, &g_wifiConsole, 5, NULL, 1);
#else // here we use serial for console
    Serial.println("Create Serial Console\r\n");
    xTaskCreatePinnedToCore(common_task, "Console", 4096, &g_SerialConsole, 5, NULL, 1); // 1 - number core so you need carefully understand number!!!  // we need implement function her 
#endif 
}

void loop() {
  // put your main code here, to run repeatedly:
    taskDelayMs(1000);
#if defined(DEBUG_MODE)
  Serial.write("Main arduino loop\r\n");
#endif
}
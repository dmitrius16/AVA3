#include <Arduino.h>
#include "modulInfo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Applications/console.h"

void common_task(void *arg) {
// now as stub
     CConsole *pTask = reinterpret_cast<CConsole*>(arg);
    if(pTask->OnInitProcess()) {
        pTask->OnTimer();
    }
}


void setup() {
  // put your setup code here, to run once:
  
  // here we need create console application  
  
  ava.init();
  xTaskCreatePinnedToCore(common_task, "Console", 4096, &g_Console, 5, NULL, 1); // 1 - number core so you need carefully understand number!!!  // we need implement function her 
}

void loop() {
  // put your main code here, to run repeatedly:
}
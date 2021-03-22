#include <Arduino.h>
#include "modulInfo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Applications/console.h"


#define DEBUG_MODE

void common_task(void *arg) {
// now as stub
#if defined(DEBUG_MODE)    
    Serial.write("Create console task");
#endif
    CConsole *pTask = reinterpret_cast<CConsole*>(arg);
    if(pTask->OnInitProcess()) {
        pTask->OnTimer();
    }
}


void setup() {
  // put your setup code here, to run once:
  
  // here we need create console application  
  Serial.begin(115200);
  #if defined(DEBUG_MODE)
    Serial.write("In void setup");
  #endif
  ava.init();
  xTaskCreatePinnedToCore(common_task, "Console", 4096, &g_Console, 5, NULL, 1); // 1 - number core so you need carefully understand number!!!  // we need implement function her 
}

void loop() {
  // put your main code here, to run repeatedly:
    taskDelayMs(1000);
#if defined(DEBUG_MODE)
  //Serial.write("Main arduino loop\r\n");
#endif
}
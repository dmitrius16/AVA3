#include <Arduino.h>
#include "modulInfo.h"
void setup() {
  // put your setup code here, to run once:
  ava.init();
  Serial.write(CModulInfo::device_name);
}

void loop() {
  // put your main code here, to run repeatedly:
}
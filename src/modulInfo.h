#pragma once

#define DEVICE_NAME "AVA3"
// OS configuration
constexpr int MAX_COUNT_MUTEX = 20; 


// end OS configuration
class CModulInfo {
public:
    static const char device_name[]; 
public:
    bool init();
};

extern CModulInfo ava;


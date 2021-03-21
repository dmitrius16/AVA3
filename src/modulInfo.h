#pragma once

#define DEVICE_NAME "AVA3"
class CModulInfo {
public:
    static const char device_name[]; 
public:
    bool init();
};

extern CModulInfo ava;


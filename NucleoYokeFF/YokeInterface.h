#pragma once
#pragma comment(lib, "hid.lib")
//#pragma comment(lib, "SetupAPI.lib")

#include "Windows.h"
#include <cstdint>
#include <hidsdi.h>
//#include <SetupAPI.h>

class YokeInterface
{
public:
    YokeInterface();
    ~YokeInterface();
    uint8_t* getSendBuffer(void) const { return const_cast<uint8_t*>(sendBuffer); }
    void open(void);
private:
    static const size_t SendBufferSize = 64;
    uint8_t sendBuffer[SendBufferSize] = { 0 };
    GUID hidGuid;       // Human Interface Device
};


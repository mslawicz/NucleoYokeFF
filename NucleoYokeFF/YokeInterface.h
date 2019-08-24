#pragma once
#include <cstdint>

class YokeInterface
{
public:
    YokeInterface();
    ~YokeInterface();
    uint8_t* getSendBuffer(void) const { return const_cast<uint8_t*>(sendBuffer); }
private:
    static const size_t SendBufferSize = 64;
    //uint8_t* pSendBuffer;
    uint8_t sendBuffer[SendBufferSize];
};


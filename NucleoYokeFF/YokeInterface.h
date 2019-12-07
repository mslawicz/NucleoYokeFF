#pragma once
#pragma comment(lib, "hid.lib")

#include "Windows.h"
#include <cstdint>
#include <hidsdi.h>

#define VENDOR_ID   0x483
#define PRODUCT_ID  0x5750  // custom HID
#define REPORT_ID   0x00    // report id 0 leaves all 64 bytes for payload
#define HID_BUFFER_SIZE    65   // report id and 64 bytes of payload


class YokeInterface
{
public:
    YokeInterface();
    ~YokeInterface();
    uint8_t* getSendBuffer(void) const { return const_cast<uint8_t*>(sendBuffer); }
    uint8_t* getRecieveBuffer(void) const { return const_cast<uint8_t*>(receiveBuffer); }
    bool openConnection(USHORT VID, USHORT PID, uint8_t collection);
    void closeConnection(void);
    void receptionEnable(void);
    bool isDataReceived(void);
    void resetReception(void) { ResetEvent(receiveOverlappedData.hEvent); } // clears the reception event (no signals until enabled again)
    void sendData(uint8_t* dataBuffer);
private:
    static const size_t ReceiveBufferSize = 260;
    uint8_t sendBuffer[HID_BUFFER_SIZE];
    uint8_t receiveBuffer[ReceiveBufferSize];
    GUID hidGuid;       // Human Interface Device
    HANDLE fileHandle;
    bool isOpen;        // true if the device is found and open
    OVERLAPPED sendOverlappedData;
    OVERLAPPED receiveOverlappedData;
    LPDWORD receivedDataCount;
    LPDWORD sendDataCount;
};


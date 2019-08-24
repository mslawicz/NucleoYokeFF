#pragma comment(lib, "SetupAPI.lib")

#include "YokeInterface.h"
#include "Logger.h"
#include <SetupAPI.h>

YokeInterface::YokeInterface()
{
    hidGuid = CLSID_NULL;
    
}

YokeInterface::~YokeInterface()
{

}

// check all connected HID devices and open Nucleo Yoke USB connection
void YokeInterface::open(void)
{
    HidD_GetHidGuid(&hidGuid);

    // SetupDiGetClassDevs function returns a handle to a device information set that contains requested device information elements for a local computer
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (deviceInfoSet != INVALID_HANDLE_VALUE)
    {
    }
    else
    {
        Logger::logMessage("Invalid handle to device information set, error code=" + std::to_string(GetLastError()));
    }
}

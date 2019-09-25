#pragma comment(lib, "SetupAPI.lib")

#include "YokeInterface.h"
#include "Logger.h"
#include <SetupAPI.h>
#include <string>
#include <cwchar>

YokeInterface::YokeInterface() :
    sendBuffer(),
    receiveBuffer(),
    sendOverlappedData(),
    receiveOverlappedData()
{
    hidGuid = CLSID_NULL;
    fileHandle = INVALID_HANDLE_VALUE;
    isOpen = false;
    memset(&sendOverlappedData, 0, sizeof(sendOverlappedData));
    sendOverlappedData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    memset(&receiveOverlappedData, 0, sizeof(receiveOverlappedData));
    receiveOverlappedData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    receivedDataCount = new DWORD;
    sendDataCount = new DWORD;
}

YokeInterface::~YokeInterface()
{
    delete receivedDataCount;
    delete sendDataCount;
}

// check all connected HID devices and open desired USB connection
bool YokeInterface::openConnection(USHORT VID, USHORT PID, uint8_t collection)
{
    bool found = false;     // mark that the device has been found
    isOpen = false;
    HidD_GetHidGuid(&hidGuid);

    // SetupDiGetClassDevs function returns a handle to a device information set that contains requested device information elements for a local computer
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
        Logger::logMessage("Invalid handle to device information set, error code=" +std::to_string(GetLastError()));
        return isOpen;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    // SetupDiEnumDeviceInfo function returns a SP_DEVINFO_DATA structure that specifies a device information element in a device information set
    for (int deviceIndex = 0; SetupDiEnumDeviceInfo(deviceInfoSet, deviceIndex, &deviceInfoData) && !found; deviceIndex++)
    {
        SP_DEVICE_INTERFACE_DATA devInterfaceData;
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        // SetupDiEnumDeviceInterfaces function enumerates the device interfaces that are contained in a device information set
        for (int interfaceIndex = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, &deviceInfoData, &hidGuid, interfaceIndex, &devInterfaceData) && !found; interfaceIndex++)
        {
            DWORD bufferSize = 0;
            // SetupDiGetDeviceInterfaceDetail function returns details about a device interface
            // this first call is for getting required size of the DeviceInterfaceDetailData buffer
            SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &devInterfaceData, NULL, 0, &bufferSize, &deviceInfoData);

            // pointer to buffer that receives information about the device that supports the requested interface
            PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(bufferSize);
            if (pDeviceInterfaceDetailData != nullptr)
            {
                pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            }
            else
            {
                Logger::logMessage("Memory allocation error");
                continue;
            }

            // SetupDiGetDeviceInterfaceDetail function returns details about a device interface
            // this second call is for getting actual information about the device that supports the requested interface
            if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &devInterfaceData, pDeviceInterfaceDetailData, bufferSize, &bufferSize, &deviceInfoData))
            {
                SECURITY_ATTRIBUTES securityAttributes;
                memset(&securityAttributes, 0, sizeof(SECURITY_ATTRIBUTES));
                securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                securityAttributes.bInheritHandle = true;

                // Creates or opens a file or I/O device
                // query metadata such as file, directory, or device attributes without accessing device 
                fileHandle = CreateFile(pDeviceInterfaceDetailData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    &securityAttributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                if (fileHandle != INVALID_HANDLE_VALUE)
                {
                    HIDD_ATTRIBUTES	attributes;
                    memset(&attributes, 0, sizeof(HIDD_ATTRIBUTES));
                    attributes.Size = sizeof(HIDD_ATTRIBUTES);
                    if (HidD_GetAttributes(fileHandle, &attributes))
                    {
                        CloseHandle(fileHandle);
                        std::wstring collectionStr = L"&col";
                        if (collection < 10)
                        {
                            collectionStr += L"0";
                        }
                        collectionStr += std::to_wstring(collection);   

                        if ((attributes.VendorID == VID) &&
                            (attributes.ProductID == PID) &&
                            (wcsstr(pDeviceInterfaceDetailData->DevicePath, collectionStr.c_str())))
                        {
                            // device with proper collection found
                            // Creates or opens a file or I/O device - this time for read/write operations in asynchronous mode
                            fileHandle = CreateFile(pDeviceInterfaceDetailData->DevicePath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                &securityAttributes, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
                            if (fileHandle != INVALID_HANDLE_VALUE)
                            {
                                isOpen = true;
                                Logger::logMessage("Connection to USB HID device with VID=" + std::to_string(VID) +" PID=" + std::to_string(PID) + " opened");
                            }
                            else
                            {
                                Logger::logMessage("USB device found, but cannot be opened for read/write operations, error code=" + std::to_string(GetLastError()));
                            }
                            // mark that the device has been found regardless if it has been opened
                            found = true;
                        }
                    }
                }
            }
            free(pDeviceInterfaceDetailData);
        }
    }
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return isOpen;
}

// closes connection to the device
void YokeInterface::closeConnection(void)
{
    if (fileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
        fileHandle = INVALID_HANDLE_VALUE;
    }
    isOpen = false;
}

// starts reception in asynchronous mode
// this way it enables reception of the incoming data
void YokeInterface::receptionEnable(void)
{
    if (isOpen && (fileHandle != INVALID_HANDLE_VALUE))
    {
        ReadFile(fileHandle, receiveBuffer, HID_BUFFER_SIZE, receivedDataCount, &receiveOverlappedData);
    }
}

// return true if received data is signaled
// this call doesn't reset the signal
bool YokeInterface::isDataReceived(void)
{
    return (WaitForSingleObject(receiveOverlappedData.hEvent, 0) == WAIT_OBJECT_0);
}

// send user data buffer in asynchronous mode
// 63 bytes are used from dataBuffer; they are sent after report id byte (total 64 bytes)
void YokeInterface::sendData(uint8_t* dataBuffer)
{
    // get overlapped result without waiting
    bool overlappedResult = GetOverlappedResult(fileHandle, &sendOverlappedData, sendDataCount, FALSE);
    DWORD lastError = GetLastError();
    // if the process is pending, return without action
    if (!overlappedResult && lastError == ERROR_IO_PENDING)
    {
        return;
    }
    // if the process is not over, but it's not pending (the other error occured)
    if (!overlappedResult)
    {
        // reset overlapped data
        memset(&sendOverlappedData, 0, sizeof(sendOverlappedData));
        sendOverlappedData.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        Logger::logMessage("USB data send error=" + std::to_string(lastError));
    }
    // send data every time if only process in not pending
    memcpy(sendBuffer + 1, dataBuffer, HID_BUFFER_SIZE - 1);
    sendBuffer[0] = REPORT_ID;
    WriteFile(fileHandle, sendBuffer, HID_BUFFER_SIZE, NULL, &sendOverlappedData);
}

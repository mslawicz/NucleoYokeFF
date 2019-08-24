#include "YokeInterface.h"

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
}

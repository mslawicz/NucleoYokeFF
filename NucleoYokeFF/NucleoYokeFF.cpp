
#include "FlightDataCollector.h"
#include "YokeInterface.h"
#include <windows.h>
#include <GL/gl.h>


#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

XPLMCreateFlightLoop_t flightLoopStructure;     // contains the parameters to create a new flight loop callback
XPLMFlightLoopID flightLoopID;      // opaque identifier for a flight loop callback

// global variables
FlightDataCollector* pXPlaneParameters = nullptr;
YokeInterface* pYokeInterface = nullptr;
// buffer for data to be sent to yoke
uint8_t dataToSend[HID_BUFFER_SIZE];

// function declarations
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{
    pXPlaneParameters = new FlightDataCollector;
    pYokeInterface = new YokeInterface;

    // set plugin signature strings
    strcpy_s(outName, 0xFF, "Nucleo Yoke Force Feedback");
    strcpy_s(outSig, 0xFF, "ms.NucleoYokeFF");
    strcpy_s(outDesc, 0xFF, "Nucleo Yoke Force Feedback plugin v4.0 for X-Plane");

    // register simulator parameters
    pXPlaneParameters->registerParameters();

    return (int)pXPlaneParameters->registrationSucceeded();
}

PLUGIN_API void	XPluginStop(void)
{
    if (pYokeInterface)
    {
        delete pYokeInterface;
    }
    if (pXPlaneParameters)
    {
        delete pXPlaneParameters;
    }
}

/* This is called when the plugin is enabled */
PLUGIN_API int  XPluginEnable(void)
{
    // open connection of the device with stated VID, PID and report_id
    pYokeInterface->openConnection(VENDOR_ID, PRODUCT_ID, REPORT_ID);
    // initialize periodic callbacks
    flightLoopStructure = { sizeof(XPLMCreateFlightLoop_t), xplm_FlightLoop_Phase_AfterFlightModel, FlightLoopCallback, nullptr };
    flightLoopID = XPLMCreateFlightLoop(&flightLoopStructure);
    XPLMScheduleFlightLoop(flightLoopID, 0.5f, 1);
    // enable reception of yoke data
    pYokeInterface->receptionEnable();
    return 1;
}

/*This is called when the plugin is disabled */
PLUGIN_API void XPluginDisable(void)
{
    // disable reception of yoke data
    pYokeInterface->resetReception();
    // stop generating periodic callbacks
    XPLMDestroyFlightLoop(flightLoopID);
    // close connection to the yoke
    pYokeInterface->closeConnection();
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) { }

/*
This function is called after per-frame X-Plane calculations; max every 10 ms
*/
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
    // read dataref values and prepare send buffer
    pXPlaneParameters->getParameters(dataToSend);
    // byte 0 is the report ID; it is not being sent if ==0
    dataToSend[0] = REPORT_ID;
    // send data to yoke
    pYokeInterface->sendData(dataToSend);

    // check if data from Nucleo Yoke has been received
    if (pYokeInterface->isDataReceived())
    {
        // data is received - set simulator parameters
        pXPlaneParameters->setParameters(pYokeInterface->getRecieveBuffer(), inElapsedSinceLastCall);
        // enable next reception from the yoke
        pYokeInterface->receptionEnable();
    }

    // return time value in seconds, after which the function will be called again
    return 0.01f;
}





#include "FlightDataCollector.h"
#include "YokeInterface.h"
#include <windows.h>
#include <GL/gl.h>


#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

XPLMCreateFlightLoop_t flightLoopStructure;     // contains the parameters to create a new flight loop callback
XPLMFlightLoopID flightLoopID;      // opaque identifier for a flight loop callback

static XPLMDataRef testTransRef;  //XXX

// global variables
FlightDataCollector* pForceFeedbackData = nullptr;
YokeInterface* pYokeInterface = nullptr;

// function declarations
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{
    pForceFeedbackData = new FlightDataCollector;
    pYokeInterface = new YokeInterface;

    // set plugin signature strings
    strcpy_s(outName, 0xFF, "Nucleo Yoke Force Feedback");
    strcpy_s(outSig, 0xFF, "ms.NucleoYokeFF");
    strcpy_s(outDesc, 0xFF, "Nucleo Yoke Force Feedback plugin for X-Plane");

    // register simulator parameters
    bool registerSuccess = true;
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/misc/act_frc_ptch_lb");
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/misc/act_frc_roll_lb");
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/misc/act_frc_hdng_lb");

    testTransRef = XPLMFindDataRef("sim/cockpit/radios/transponder_code"); //XXX

    return (int)registerSuccess;
}

PLUGIN_API void	XPluginStop(void)
{
    if (pYokeInterface)
    {
        delete pYokeInterface;
    }
    if (pForceFeedbackData)
    {
        delete pForceFeedbackData;
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
    XPLMScheduleFlightLoop(flightLoopID, 5.0f, 1);
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

float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
    // this test reads reads data from the yoke and send them back to yoke
    static uint8_t cnt = 0;
    if (pYokeInterface->isDataReceived())
    {
        // data is received
        // first byte (after the report_id) is the callback counter
        uint8_t dataToSend[63] = { cnt++ };
        // next copy 62 bytes from receive buffer
        memcpy(dataToSend + 1, pYokeInterface->getRecieveBuffer(), 62);
        // send data to yoke
        pYokeInterface->sendData(dataToSend);
        // enable next reception from the yoke
    }
 
    // returned value >0 means the time in seconds, after which the function is called again
    return 0.01f;
}
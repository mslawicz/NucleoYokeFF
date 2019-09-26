
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
    // pitch force to yoke
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/misc/act_frc_ptch_lb");
    // roll force to yoke
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/misc/act_frc_roll_lb");
    // yaw force to yoke
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/misc/act_frc_hdng_lb");
    // deflection of flaps 0..1
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/controls/flaprat");
    // int/bool are there any retracted gears?
    registerSuccess &= pForceFeedbackData->registerParameter("sim/aircraft/gear/acf_gear_retract");
    // gear 1 deflection
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/movingparts/gear1def");
    // gear 2 deflection
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/movingparts/gear2def");
    // gear 3 deflection
    registerSuccess &= pForceFeedbackData->registerParameter("sim/flightmodel/movingparts/gear3def");

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
    XPLMScheduleFlightLoop(flightLoopID, 1.0f, 1);
    // enable reception of yoke data
    //pYokeInterface->receptionEnable();
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
    static uint8_t cnt = 0;

    // buffer for data to be sent to yoke
    static uint8_t dataToSend[HID_BUFFER_SIZE];
    // first byte of data is the frame counter
    dataToSend[0] = cnt++;
    // read registered parameters and place them in the buffer
    pForceFeedbackData->readParameters(dataToSend + 1);
    // send data to yoke
    pYokeInterface->sendData(dataToSend);
 
    // returned value >0 means the time in seconds, after which the function will be called again
    return 0.02f;
}
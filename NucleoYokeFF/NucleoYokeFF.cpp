
#include "FlightDataCollector.h"
#include <windows.h>
#include <GL/gl.h>


#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

XPLMCreateFlightLoop_t flightLoopStructure;     // contains the parameters to create a new flight loop callback
XPLMFlightLoopID flightLoopID;      // opaque identifier for a flight loop callback

static XPLMDataRef testTransRef;  //XXX
uint8_t dataBuffer[64];//XXX

// global variables
FlightDataCollector* pForceFeedbackData = nullptr;

// function declarations
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{
    pForceFeedbackData = new FlightDataCollector;

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

    if (!registerSuccess)
    {
        strcpy_s(outDesc, 0xFF, "parameter registration failed");
    }
    return (int)registerSuccess;
}

PLUGIN_API void	XPluginStop(void)
{
    if (pForceFeedbackData)
    {
        delete pForceFeedbackData;
    }
}

/* This is called when the plugin is enabled */
PLUGIN_API int  XPluginEnable(void)
{
    flightLoopStructure = { sizeof(XPLMCreateFlightLoop_t), xplm_FlightLoop_Phase_AfterFlightModel, FlightLoopCallback, nullptr };
    flightLoopID = XPLMCreateFlightLoop(&flightLoopStructure);
    XPLMScheduleFlightLoop(flightLoopID, 5.0f, 1);
    return 1;
}

/*This is called when the plugin is disabled */
PLUGIN_API void XPluginDisable(void)
{
    XPLMDestroyFlightLoop(flightLoopID);
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) { }

float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
    // this test reads yoke roll feedback force and set this value to the transponder 
    pForceFeedbackData->readParameters(dataBuffer);
    float rollForce = *reinterpret_cast<float*>(dataBuffer+4);
    XPLMSetDatai(testTransRef, 2000 + (int)rollForce);

    // returned value >0 means the time in seconds, after which the function is called again
    return 0.5f;
}
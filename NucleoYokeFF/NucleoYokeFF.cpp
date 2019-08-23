
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include <cmath>

#if IBM
#include <windows.h>
#endif
#if LIN
#include <GL/gl.h>
#elif __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif

XPLMCreateFlightLoop_t flightLoopStructure;
XPLMFlightLoopID flightLoopID;
static XPLMDataRef testForceRef;  //XXX
static XPLMDataRef testTransRef;  //XXX

float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);

PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{
    strcpy_s(outName, 0xFF, "Nucleo Yoke Force Feedback");
    strcpy_s(outSig, 0xFF, "ms.NucleoYokeFF");
    strcpy_s(outDesc, 0xFF, "Nucleo Yoke Force Feedback plugin for X-Plane");

    testForceRef = XPLMFindDataRef("sim/flightmodel/misc/act_frc_roll_lb");
    testTransRef = XPLMFindDataRef("sim/cockpit/radios/transponder_code");

    return 1;
}

PLUGIN_API void	XPluginStop(void)
{

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
    float rollForce = XPLMGetDataf(testForceRef);
    XPLMSetDatai(testTransRef, 2000 + (int)rollForce);

    // returned value >0 means the time in seconds, after which the function is called again
    return 0.5f;
}
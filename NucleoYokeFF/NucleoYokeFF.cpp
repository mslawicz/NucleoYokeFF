
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
    strcpy_s(outDesc, 0xFF, "Nucleo Yoke Force Feedback plugin v2.0 for X-Plane");

    // register simulator parameters
    bool registerSuccess = true;
    // int/bool are there any retracted gears?
    registerSuccess &= pForceFeedbackData->registerParameter("is_retractable", "sim/aircraft/gear/acf_gear_retract");
    // gear 1 deflection <0.0f .. 1.0f>
    registerSuccess &= pForceFeedbackData->registerParameter("nose_gear_deflection", "sim/flightmodel/movingparts/gear1def");
    // gear 2 deflection <0.0f .. 1.0f>
    registerSuccess &= pForceFeedbackData->registerParameter("left_gear_deflection", "sim/flightmodel/movingparts/gear2def");
    // gear 3 deflection <0.0f .. 1.0f>
    registerSuccess &= pForceFeedbackData->registerParameter("right_gear_deflection", "sim/flightmodel/movingparts/gear3def");
	// deflection of flaps <0.0f .. 1.0f>
	registerSuccess &= pForceFeedbackData->registerParameter("flaps_deflection", "sim/flightmodel/controls/flaprat");
	// Total pitch control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
	registerSuccess &= pForceFeedbackData->registerParameter("total_pitch", "sim/cockpit2/controls/total_pitch_ratio");
	// Total roll control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
	registerSuccess &= pForceFeedbackData->registerParameter("total_roll", "sim/cockpit2/controls/total_roll_ratio");
	// Total yaw control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
	registerSuccess &= pForceFeedbackData->registerParameter("total_yaw", "sim/cockpit2/controls/total_heading_ratio");
	// Throttle position of the handle itself - this controls all the handles at once. <0.0f .. 1.0f>
	registerSuccess &= pForceFeedbackData->registerParameter("throttle", "sim/cockpit2/engine/actuators/throttle_ratio_all");
	// Maximum structural cruising speed or maximum speed for normal operations [kias]
	registerSuccess &= pForceFeedbackData->registerParameter("acf_vno", "sim/aircraft/view/acf_Vno");
	// Air speed indicated - this takes into account air density and wind direction [kias]
	registerSuccess &= pForceFeedbackData->registerParameter("indicated_airspeed", "sim/flightmodel/position/indicated_airspeed");
	// Gear/ground forces - sideways - ACF X. Override with override_gear_forces [Newtons]
	registerSuccess &= pForceFeedbackData->registerParameter("gear_force_sideways", "sim/flightmodel/forces/fside_gear");
	// Gear/ground forces - upward - ACF Y [Newtons]
	registerSuccess &= pForceFeedbackData->registerParameter("gear_force_upward", "sim/flightmodel/forces/fnrml_gear");
	// Gear/ground forces - backward - ACF Z [Newtons]
	registerSuccess &= pForceFeedbackData->registerParameter("gear_force_backward", "sim/flightmodel/forces/faxil_gear");

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
	float fParameter;
	int iParameter;

    // buffer for data to be sent to yoke
    static uint8_t dataToSend[HID_BUFFER_SIZE];

    // byte 0 is the report ID
	dataToSend[0] = REPORT_ID;

	// byte 1 is the frame counter
	dataToSend[1] = cnt++;

	// byte 2 is the simulator boolean flag register
	dataToSend[2] = 0;

	// set 'is retractable' flag
	if (pForceFeedbackData->readInt("is_retractable") != 0)
	{
		dataToSend[2] |= (1 << 0);
	}

	// byte 3 is gear deflection state (3 gears)
	// every gear is coded in 2 bits: 0-fully up, 1-under way, 2-fully down, 3-not used
	fParameter = pForceFeedbackData->readFloat("nose_gear_deflection");
	if (fParameter == 1.0f)
	{
		dataToSend[3] |= (0x02 << 0);
	}
	else if (fParameter > 0.0f)
	{
		dataToSend[3] |= (0x01 << 0);
	}

	fParameter = pForceFeedbackData->readFloat("left_gear_deflection");
	if (fParameter == 1.0f)
	{
		dataToSend[3] |= (0x02 << 2);
	}
	else if (fParameter > 0.0f)
	{
		dataToSend[3] |= (0x01 << 2);
	}

	fParameter = pForceFeedbackData->readFloat("right_gear_deflection");
	if (fParameter == 1.0f)
	{
		dataToSend[3] |= (0x02 << 4);
	}
	else if (fParameter > 0.0f)
	{
		dataToSend[3] |= (0x01 << 4);
	}

	// bytes 4-7 is flaps deflection <0.0f .. 1.0f>
	fParameter = pForceFeedbackData->readFloat("flaps_deflection");
	memcpy(dataToSend + 4, &fParameter, sizeof(fParameter));

	// bytes 8-11 is total pitch control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
	fParameter = pForceFeedbackData->readFloat("total_pitch");
	memcpy(dataToSend + 8, &fParameter, sizeof(fParameter));

	// bytes 12-15 is total roll control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
	fParameter = pForceFeedbackData->readFloat("total_roll");
	memcpy(dataToSend + 12, &fParameter, sizeof(fParameter));

	// bytes 16-19 is total yaw control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
	fParameter = pForceFeedbackData->readFloat("total_yaw");
	memcpy(dataToSend + 16, &fParameter, sizeof(fParameter));

	// bytes 20-23 is throttle position of the handle itself - this controls all the handles at once <0.0f .. 1.0f>
	fParameter = pForceFeedbackData->readFloat("throttle");
	memcpy(dataToSend + 20, &fParameter, sizeof(fParameter));

	// bytes 24-27 is aircraft airspeed in relation to its Vno <0.0f .. 1.0f+> (may exceed 1.0f)
	fParameter = pForceFeedbackData->readFloat("indicated_airspeed") / pForceFeedbackData->readFloat("acf_vno");
	memcpy(dataToSend + 24, &fParameter, sizeof(fParameter));

    // send data to yoke
    pYokeInterface->sendData(dataToSend);
 
    // returned value >0 means the time in seconds, after which the function will be called again
    return 0.02f;
}
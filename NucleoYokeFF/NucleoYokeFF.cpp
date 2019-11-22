
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

// function declarations
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon);
void setParameters(uint8_t* receiveBuffer);
void getParameters(void);

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
    strcpy_s(outDesc, 0xFF, "Nucleo Yoke Force Feedback plugin v2.0 for X-Plane");

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
    flightLoopStructure = { sizeof(XPLMCreateFlightLoop_t), xplm_FlightLoop_Phase_BeforeFlightModel, FlightLoopCallback, nullptr };
    flightLoopID = XPLMCreateFlightLoop(&flightLoopStructure);
    XPLMScheduleFlightLoop(flightLoopID, 1.0f, 1);
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
This function is called before per-frame X-Plane calculations; max every 10 ms
*/
float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon)
{
    // read dataref values and prepare send buffer
    getParameters();

    // check if data from Nucleo Yoke has been received
    if (pYokeInterface->isDataReceived())
    {
        // data is received - set simulator parameters
        setParameters(pYokeInterface->getRecieveBuffer());
        // enable next reception from the yoke
        pYokeInterface->receptionEnable();
    }

    // return time value in seconds, after which the function will be called again
    return 0.01f;
}


// set simulator parameters according to received data
void setParameters(uint8_t* receiveBuffer)
{
    // set yoke pitch from received bytes 8-11
    XPLMSetDataf(pXPlaneParameters->getHandle("yoke_pitch"), *reinterpret_cast<float*>(receiveBuffer + 8));
    // set yoke roll from received bytes 12-15
    XPLMSetDataf(pXPlaneParameters->getHandle("yoke_roll"), *reinterpret_cast<float*>(receiveBuffer + 12));
    // set 'pedals' deflection from received bytes 16-19
    XPLMSetDataf(pXPlaneParameters->getHandle("yoke_heading"), *reinterpret_cast<float*>(receiveBuffer + 16));
    // set throttle from received bytes 20-23
    XPLMSetDataf(pXPlaneParameters->getHandle("throttle"), *reinterpret_cast<float*>(receiveBuffer + 20));
    // set mixture from received bytes 24-27
    XPLMSetDataf(pXPlaneParameters->getHandle("mixture"), *reinterpret_cast<float*>(receiveBuffer + 24));
    // set propeller from received bytes 28-31
    float propellerMin = XPLMGetDataf(pXPlaneParameters->getHandle("prop_min"));
    float propellerMax = XPLMGetDataf(pXPlaneParameters->getHandle("prop_max"));
    float propellerRotationSpeed = propellerMin + (*reinterpret_cast<float*>(receiveBuffer + 28)) * (propellerMax - propellerMin);
    XPLMSetDataf(pXPlaneParameters->getHandle("propeller"), propellerRotationSpeed);
    //XXX set transponder for test
    XPLMSetDatai(pXPlaneParameters->getHandle("transponder"), ((*reinterpret_cast<int*>(receiveBuffer + 4)) & 0xFF) + 2000);
}

// get simulator parameters and send them to yoke in report id 3 frame
void getParameters(void)
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
    if (XPLMGetDatai(pXPlaneParameters->getHandle("is_retractable")) != 0)
    {
        dataToSend[2] |= (1 << 0);
    }

    // byte 3 is gear deflection state (3 gears)
    // every gear is coded in 2 bits: 0-fully up, 1-under way, 2-fully down, 3-not used
    dataToSend[3] = 0;
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("nose_gear_deflection"));
    if (fParameter == 1.0f)
    {
        dataToSend[3] |= (0x02 << 0);
    }
    else if (fParameter > 0.0f)
    {
        dataToSend[3] |= (0x01 << 0);
    }

    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("left_gear_deflection"));
    if (fParameter == 1.0f)
    {
        dataToSend[3] |= (0x02 << 2);
    }
    else if (fParameter > 0.0f)
    {
        dataToSend[3] |= (0x01 << 2);
    }

    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("right_gear_deflection"));
    if (fParameter == 1.0f)
    {
        dataToSend[3] |= (0x02 << 4);
    }
    else if (fParameter > 0.0f)
    {
        dataToSend[3] |= (0x01 << 4);
    }

    // bytes 4-7 is flaps deflection <0.0f .. 1.0f>
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("flaps_deflection"));
    memcpy(dataToSend + 4, &fParameter, sizeof(fParameter));

    // bytes 8-11 is total pitch control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("total_pitch"));
    memcpy(dataToSend + 8, &fParameter, sizeof(fParameter));

    // bytes 12-15 is total roll control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("total_roll"));
    memcpy(dataToSend + 12, &fParameter, sizeof(fParameter));

    // bytes 16-19 is total yaw control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("total_yaw"));
    memcpy(dataToSend + 16, &fParameter, sizeof(fParameter));

    // bytes 20-23 is throttle position of the handle itself - this controls all the handles at once <0.0f .. 1.0f>
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("throttle"));
    memcpy(dataToSend + 20, &fParameter, sizeof(fParameter));

    // bytes 24-27 is aircraft airspeed in relation to its Vno <0.0f .. 1.0f+> (may exceed 1.0f)
    fParameter = XPLMGetDataf(pXPlaneParameters->getHandle("indicated_airspeed")) / XPLMGetDataf(pXPlaneParameters->getHandle("acf_vno"));
    memcpy(dataToSend + 24, &fParameter, sizeof(fParameter));

    if ((XPLMGetDatai(pXPlaneParameters->getHandle("stick_shaker")) != 0) &&
        (XPLMGetDatai(pXPlaneParameters->getHandle("stall_warning")) != 0))
    {
        // this aircraft is equipped with a stick shaker and 
        dataToSend[2] |= (1 << 1);
    }

    if (XPLMGetDatai(pXPlaneParameters->getHandle("reverser_deployed")) != 0)
    {
        // reverser is on
        dataToSend[2] |= (1 << 2);
    }

    // bytes 28-31 is propeller speed in [rpm]; the higher value of first 2 engines is used
    float propSpeed[2];
    if (XPLMGetDatavf(pXPlaneParameters->getHandle("prop_speed"), propSpeed, 0, 2) == 2)
    {
        fParameter = propSpeed[0] > propSpeed[1] ? propSpeed[0] : propSpeed[1];
        memcpy(dataToSend + 28, &fParameter, sizeof(fParameter));
    }

    // send data to yoke
    pYokeInterface->sendData(dataToSend);
}
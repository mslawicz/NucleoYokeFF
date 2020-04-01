#include "Logger.h"
#include "FlightDataCollector.h"
#include "YokeInterface.h"

FlightDataCollector::FlightDataCollector(void)
{
    success = true;
}

/*
* register DataRef parameter to be inquired
*/
void FlightDataCollector::registerParameter(std::string parameterNickname, std::string parameterName)
{
    SimulatorParameter newParameter;
    newParameter.name = parameterName;
    newParameter.handle = XPLMFindDataRef(newParameter.name.c_str());
    newParameter.type = XPLMGetDataRefTypes(newParameter.handle);

    if (newParameter.handle)
    {
		simulatorParameters.emplace(parameterNickname, newParameter);
    }
    else
    {
        Logger::logMessage("failed to register parameter " + parameterName);
        success = false;
    }
}

/*
* returns the handle of the parameter
*/
XPLMDataRef FlightDataCollector::getHandle(std::string parameterNickname) const
{
    auto parameterIt = simulatorParameters.find(parameterNickname);
    if (parameterIt == simulatorParameters.end())
    {
        Logger::logMessage("invalid parameter nickname: " + parameterNickname);
        return nullptr;
    }
    else
    {
        return parameterIt->second.handle;
    }
}

/*
* register all needed flight simulator parameters
*/
void FlightDataCollector::registerParameters(void)
{
    // This is how much the user input has deflected the yoke in the cockpit, in ratio, where -1.0 is full down, and 1.0 is full up
    registerParameter("yoke_pitch", "sim/cockpit2/controls/yoke_pitch_ratio");
    // This is how much the user input has deflected the yoke in the cockpit, in ratio, where -1.0 is full left, and 1.0 is full right.
    registerParameter("yoke_roll", "sim/cockpit2/controls/yoke_roll_ratio");
    // This is how much the user input has deflected the rudder in the cockpit, in ratio, where -1.0 is full left, and 1.0 is full right
    registerParameter("yoke_heading", "sim/cockpit2/controls/yoke_heading_ratio");
    // int/bool are there any retracted gears?
    registerParameter("is_retractable", "sim/aircraft/gear/acf_gear_retract");
    // gear 1 deflection <0.0f .. 1.0f>
    registerParameter("nose_gear_deflection", "sim/flightmodel/movingparts/gear1def");
    // gear 2 deflection <0.0f .. 1.0f>
    registerParameter("left_gear_deflection", "sim/flightmodel/movingparts/gear2def");
    // gear 3 deflection <0.0f .. 1.0f>
    registerParameter("right_gear_deflection", "sim/flightmodel/movingparts/gear3def");
    // deflection of flaps <0.0f .. 1.0f>
    registerParameter("flaps_deflection", "sim/flightmodel/controls/flaprat");
    // Total pitch control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    registerParameter("total_pitch", "sim/cockpit2/controls/total_pitch_ratio");
    // Total roll control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    registerParameter("total_roll", "sim/cockpit2/controls/total_roll_ratio");
    // Total yaw control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    registerParameter("total_yaw", "sim/cockpit2/controls/total_heading_ratio");
    // Throttle position of the handle itself - this controls all the handles at once. <0.0f .. 1.0f>
    registerParameter("throttle", "sim/cockpit2/engine/actuators/throttle_ratio_all");
    // Maximum structural cruising speed or maximum speed for normal operations [kias]
    registerParameter("acf_vno", "sim/aircraft/view/acf_Vno");
    // Air speed indicated - this takes into account air density and wind direction [kias]
    registerParameter("indicated_airspeed", "sim/flightmodel/position/indicated_airspeed");
    // stick shaker available?
    registerParameter("stick_shaker", "sim/aircraft/forcefeedback/acf_ff_stickshaker");
    // stall warning on?
    registerParameter("stall_warning", "sim/cockpit2/annunciators/stall_warning");
    // reverserser on (one bit for each engine)
    registerParameter("reverser_deployed", "sim/cockpit2/annunciators/reverser_deployed");
    // Prop speed float array for max 8 engines [rpm]
    registerParameter("prop_speed", "sim/cockpit2/engine/indicators/prop_speed_rpm");
    // Throttle position of the handle itself - this controls all the handles at once
    registerParameter("throttle", "sim/cockpit2/engine/actuators/throttle_ratio_all");
    // Mixture handle position, this controls all at once
    registerParameter("mixture", "sim/cockpit2/engine/actuators/mixture_ratio_all");
    // Prop handle position, this controls all props at once.
    registerParameter("propeller", "sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all");
    // Minimum prop speed with governor on, radians/second
    registerParameter("prop_min", "sim/aircraft/controls/acf_RSC_mingov_prp");
    // Max prop speed radians/second
    registerParameter("prop_max", "sim/aircraft/controls/acf_RSC_redline_prp");
    // flap detents
    registerParameter("flap_detents", "sim/aircraft/controls/acf_flap_detents");
    // requested flap ratio
    registerParameter("flap_request", "sim/flightmodel/controls/flaprqst");
    // Elevator trim, in part of MAX FLIGHT CONTROL DEFLECTION
    registerParameter("elevator_trim", "sim/cockpit2/controls/elevator_trim");
    // total time taken for the elevator trim to go from one extreme to the other [s]
    registerParameter("elevator_trim_time", "sim/aircraft/controls/acf_elev_trim_time");
    // Gear handle position. 0 is up. 1 is down
    registerParameter("gear", "sim/cockpit2/controls/gear_handle_down");
    // Position of pilot's head heading
    registerParameter("head_yaw", "sim/graphics/view/pilots_head_psi");
    // Position of pilot's head pitch
    registerParameter("head_pitch", "sim/graphics/view/pilots_head_the");
}


/*
* get parameters value and place them in the send buffer
*/
void FlightDataCollector::getParameters(uint8_t* dataToSend)
{
    static uint8_t cnt = 0;
    float fParameter;

    // bytes 1-2 is the simulator boolean flag register
    dataToSend[1] = dataToSend[2] = 0;

    // set 'is retractable' flag
    if (XPLMGetDatai(getHandle("is_retractable")) != 0)
    {
        dataToSend[1] |= (1 << 0);
    }

    // byte 3 is gear deflection state (3 gears)
    // every gear is coded in 2 bits: 0-fully up, 1-under way, 2-fully down, 3-not used
    dataToSend[3] = 0;
    fParameter = XPLMGetDataf(getHandle("nose_gear_deflection"));
    if (fParameter == 1.0f)
    {
        dataToSend[3] |= (0x02 << 0);
    }
    else if (fParameter > 0.0f)
    {
        dataToSend[3] |= (0x01 << 0);
    }

    fParameter = XPLMGetDataf(getHandle("left_gear_deflection"));
    if (fParameter == 1.0f)
    {
        dataToSend[3] |= (0x02 << 2);
    }
    else if (fParameter > 0.0f)
    {
        dataToSend[3] |= (0x01 << 2);
    }

    fParameter = XPLMGetDataf(getHandle("right_gear_deflection"));
    if (fParameter == 1.0f)
    {
        dataToSend[3] |= (0x02 << 4);
    }
    else if (fParameter > 0.0f)
    {
        dataToSend[3] |= (0x01 << 4);
    }

    // bytes 4-7 is flaps deflection <0.0f .. 1.0f>
    fParameter = XPLMGetDataf(getHandle("flaps_deflection"));
    memcpy(dataToSend + 4, &fParameter, sizeof(fParameter));

    // bytes 8-11 is total pitch control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    fParameter = XPLMGetDataf(getHandle("total_pitch"));
    memcpy(dataToSend + 8, &fParameter, sizeof(fParameter));

    // bytes 12-15 is total roll control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    fParameter = XPLMGetDataf(getHandle("total_roll"));
    memcpy(dataToSend + 12, &fParameter, sizeof(fParameter));

    // bytes 16-19 is total yaw control input (sum of user yoke plus autopilot servo plus artificial stability) <-1.0f .. 1.0f>
    fParameter = XPLMGetDataf(getHandle("total_yaw"));
    memcpy(dataToSend + 16, &fParameter, sizeof(fParameter));

    // bytes 20-23 is throttle position of the handle itself - this controls all the handles at once <0.0f .. 1.0f>
    fParameter = XPLMGetDataf(getHandle("throttle"));
    memcpy(dataToSend + 20, &fParameter, sizeof(fParameter));

    // bytes 24-27 is aircraft airspeed in relation to its Vno <0.0f .. 1.0f+> (may exceed 1.0f)
    fParameter = XPLMGetDataf(getHandle("indicated_airspeed")) / XPLMGetDataf(getHandle("acf_vno"));
    memcpy(dataToSend + 24, &fParameter, sizeof(fParameter));

    if ((XPLMGetDatai(getHandle("stick_shaker")) != 0) &&
        (XPLMGetDatai(getHandle("stall_warning")) != 0))
    {
        // this aircraft is equipped with a stick shaker and 
        dataToSend[2] |= (1 << 1);
    }

    if (XPLMGetDatai(getHandle("reverser_deployed")) != 0)
    {
        // reverser is on
        dataToSend[2] |= (1 << 2);
    }

    // bytes 28-31 is propeller speed in [rpm]; the higher value of first 2 engines is used
    float propSpeed[2];
    if (XPLMGetDatavf(getHandle("prop_speed"), propSpeed, 0, 2) == 2)
    {
        fParameter = propSpeed[0] > propSpeed[1] ? propSpeed[0] : propSpeed[1];
        memcpy(dataToSend + 28, &fParameter, sizeof(fParameter));
    }
}

/*
set simulator parameters from received data from yoke
*/
void FlightDataCollector::setParameters(uint8_t* receiveBuffer, float timeElapsed)
{

}



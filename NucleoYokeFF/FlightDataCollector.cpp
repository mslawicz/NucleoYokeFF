#include "Logger.h"
#include "FlightDataCollector.h"

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
    // XXX transponder for test purposes
    registerParameter("transponder", "sim/cockpit/radios/transponder_code");
}



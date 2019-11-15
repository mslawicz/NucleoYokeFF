#pragma once

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include <string>
#include <unordered_map>

struct SimulatorParameter
{
    XPLMDataRef handle;
    std::string name;
    XPLMDataTypeID type;
};

class FlightDataCollector
{
public:
    FlightDataCollector(void);
    void registerParameter(std::string parameterNickname, std::string parameterName);
    bool registrationSucceeded(void) const { return success; }
    XPLMDataRef getHandle(std::string parameterNickname) const;
private:
    std::unordered_map<std::string, SimulatorParameter> simulatorParameters;
    bool success;
};


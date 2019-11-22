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
    bool registrationSucceeded(void) const { return success; }
    XPLMDataRef getHandle(std::string parameterNickname) const;
    void registerParameters(void);
    void getParameters(uint8_t* dataToSend);
private:
    void registerParameter(std::string parameterNickname, std::string parameterName);
    std::unordered_map<std::string, SimulatorParameter> simulatorParameters;
    bool success;
};


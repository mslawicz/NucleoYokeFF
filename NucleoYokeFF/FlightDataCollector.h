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
    bool registerParameter(std::string parameterNickname, std::string parameterName);
	float readFloat(std::string parameterNickname);
	int readInt(std::string parameterNickname);
private:
    std::unordered_map<std::string, SimulatorParameter> simulatorParameters;
};


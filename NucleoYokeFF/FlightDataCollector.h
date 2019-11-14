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
    int readIntArray(std::string parameterNickname, int* buffer, int length, int offset = 0);
    void writeInt(std::string parameterNickname, int value);
private:
    std::unordered_map<std::string, SimulatorParameter> simulatorParameters;
};


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
	float readFloat(std::string parameterNickname);
	int readInt(std::string parameterNickname);
    int readIntArray(std::string parameterNickname, int* buffer, int length, int offset = 0);
    int readFloatArray(std::string parameterNickname, float* buffer, int length, int offset = 0);
    void writeInt(std::string parameterNickname, int value);
private:
    std::unordered_map<std::string, SimulatorParameter> simulatorParameters;
    bool success;
};


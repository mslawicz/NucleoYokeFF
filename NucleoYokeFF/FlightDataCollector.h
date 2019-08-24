#pragma once

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include <string>
#include <vector>

struct SimulatorParameter
{
    XPLMDataRef handle;
    std::string name;
    XPLMDataTypeID type;
};

class FlightDataCollector
{
public:
    bool registerParameter(std::string parameterName);
    void readParameters(uint8_t* buffer);
private:
    std::vector<SimulatorParameter> simulatorParameters;
};


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
    bool isBoolean;
};

class FlightDataCollector
{
public:
    FlightDataCollector(void);
    bool registerParameter(std::string parameterName, bool isBoolean = false);
    void readParameters(uint8_t* buffer);
    uint32_t getBooleanFlags(void) const { return booleanFlags; }
private:
    std::vector<SimulatorParameter> simulatorParameters;
    uint32_t booleanFlags;      // flag register for all boolean parameters
    uint32_t booleanMask;       // mask for setting boolean flags
};


#include "Logger.h"
#include "FlightDataCollector.h"

bool FlightDataCollector::registerParameter(std::string parameterName)
{
    bool result = false;
    SimulatorParameter newParameter;
    newParameter.name = parameterName;
    newParameter.handle = XPLMFindDataRef(newParameter.name.c_str());
    newParameter.type = XPLMGetDataRefTypes(newParameter.handle);

    if (newParameter.handle)
    {
        simulatorParameters.push_back(newParameter);
        result = true;
    }
    else
    {
        Logger::logMessage("failed to register parameter " + parameterName);
    }
    return result;
}

void FlightDataCollector::readParameters(uint8_t* buffer)
{
    // iterate over vector of registered parameters
    for (auto const& parameter : simulatorParameters)
    {
        // action dependent on parameter type
        switch (parameter.type)
        {
        case xplmType_Float:
        {
            auto value = XPLMGetDataf(parameter.handle);
            memcpy(buffer, &value, sizeof(value));
            buffer += sizeof(value);
            break;
        }
        default:
            break;
        }
    }
}

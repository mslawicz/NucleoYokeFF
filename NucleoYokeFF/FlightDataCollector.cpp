#include "Logger.h"
#include "FlightDataCollector.h"

FlightDataCollector::FlightDataCollector(void)
{
    booleanFlags = 0;
    booleanMask = 0x00000001;
}

/*
* register DataRef parameter to be inquired
* if isBoolean==true, the parameter is treated as boolean
*/
bool FlightDataCollector::registerParameter(std::string parameterName, bool isBoolean)
{
    bool result = false;
    SimulatorParameter newParameter;
    newParameter.name = parameterName;
    newParameter.handle = XPLMFindDataRef(newParameter.name.c_str());
    newParameter.type = XPLMGetDataRefTypes(newParameter.handle);
    newParameter.isBoolean = isBoolean;

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

// read all registered parameters and place their values in the buffer
void FlightDataCollector::readParameters(uint8_t* buffer)
{
    booleanFlags = 0;
    booleanMask = 0x00000001;
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
        case xplmType_Int:
        {
            auto value = XPLMGetDatai(parameter.handle);
            if (parameter.isBoolean)
            {
                // this parameter is boolean
                if (value != 0)
                {
                    booleanFlags |= booleanMask;
                }
                booleanMask <<= 1;
            }
            else
            {
                // it is regular int parameter
                memcpy(buffer, &value, sizeof(value));
                buffer += sizeof(value);
            }
            break;
        }
        default:
            break;
        }
    }
}

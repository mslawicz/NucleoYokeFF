#include "Logger.h"
#include "FlightDataCollector.h"

FlightDataCollector::FlightDataCollector(void)
{

}

/*
* register DataRef parameter to be inquired
*/
bool FlightDataCollector::registerParameter(std::string parameterNickname, std::string parameterName)
{
    bool result = false;
    SimulatorParameter newParameter;
    newParameter.name = parameterName;
    newParameter.handle = XPLMFindDataRef(newParameter.name.c_str());
    newParameter.type = XPLMGetDataRefTypes(newParameter.handle);

    if (newParameter.handle)
    {
		simulatorParameters.emplace(parameterNickname, newParameter);
        result = true;
    }
    else
    {
        Logger::logMessage("failed to register parameter " + parameterName);
    }
    return result;
}

/*
* read float parameter
*/
float FlightDataCollector::readFloat(std::string parameterNickname)
{
	auto parameterIt = simulatorParameters.find(parameterNickname);
	if ((parameterIt == simulatorParameters.end()) || (parameterIt->second.type != xplmType_Float))
	{
		return 0.0f;
	}
	else
	{
		return XPLMGetDataf(parameterIt->second.handle);
	}
}

/*
* read int parameter
*/
int FlightDataCollector::readInt(std::string parameterNickname)
{
	auto parameterIt = simulatorParameters.find(parameterNickname);
	if ((parameterIt == simulatorParameters.end()) || (parameterIt->second.type != xplmType_Int))
	{
		return 0;
	}
	else
	{
		return XPLMGetDatai(parameterIt->second.handle);
	}
}

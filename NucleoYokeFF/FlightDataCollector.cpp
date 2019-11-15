#include "Logger.h"
#include "FlightDataCollector.h"

FlightDataCollector::FlightDataCollector(void)
{
    success = true;
}

/*
* register DataRef parameter to be inquired
*/
void FlightDataCollector::registerParameter(std::string parameterNickname, std::string parameterName)
{
    SimulatorParameter newParameter;
    newParameter.name = parameterName;
    newParameter.handle = XPLMFindDataRef(newParameter.name.c_str());
    newParameter.type = XPLMGetDataRefTypes(newParameter.handle);

    if (newParameter.handle)
    {
		simulatorParameters.emplace(parameterNickname, newParameter);
    }
    else
    {
        Logger::logMessage("failed to register parameter " + parameterName);
        success = false;
    }
}

/*
* returns the handle of the parameter
*/
XPLMDataRef FlightDataCollector::getHandle(std::string parameterNickname) const
{
    auto parameterIt = simulatorParameters.find(parameterNickname);
    if (parameterIt == simulatorParameters.end())
    {
        Logger::logMessage("invalid parameter nickname: " + parameterNickname);
        return nullptr;
    }
    else
    {
        return parameterIt->second.handle;
    }
}


		//return XPLMGetDataf(parameterIt->second.handle);
		//return XPLMGetDatai(parameterIt->second.handle);
  //      return XPLMGetDatavi(parameterIt->second.handle, buffer, offset, length);
  //      return XPLMGetDatavf(parameterIt->second.handle, buffer, offset, length);
  //      XPLMSetDatai(parameterIt->second.handle, value);

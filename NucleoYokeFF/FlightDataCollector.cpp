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

/*
* read array of int parameter
*/
int FlightDataCollector::readIntArray(std::string parameterNickname, int* buffer, int length, int offset)
{
    auto parameterIt = simulatorParameters.find(parameterNickname);
    if ((parameterIt == simulatorParameters.end()) || (parameterIt->second.type != xplmType_IntArray))
    {
        return 0;
    }
    else
    {
        return XPLMGetDatavi(parameterIt->second.handle, buffer, offset, length);
    }
}

/*
* read array of float parameter
*/
int FlightDataCollector::readFloatArray(std::string parameterNickname, float* buffer, int length, int offset)
{
    auto parameterIt = simulatorParameters.find(parameterNickname);
    if ((parameterIt == simulatorParameters.end()) || (parameterIt->second.type != xplmType_FloatArray))
    {
        return 0;
    }
    else
    {
        return XPLMGetDatavf(parameterIt->second.handle, buffer, offset, length);
    }
}

/*
* write int parameter
*/
void FlightDataCollector::writeInt(std::string parameterNickname, int value)
{
    auto parameterIt = simulatorParameters.find(parameterNickname);
    if ((parameterIt == simulatorParameters.end()) || (parameterIt->second.type != xplmType_Int))
    {
        return;
    }
    else
    {
        XPLMSetDatai(parameterIt->second.handle, value);
    }
}
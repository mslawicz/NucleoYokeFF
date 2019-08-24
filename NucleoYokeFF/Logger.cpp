#include "Logger.h"
#include "XPLMUtilities.h"

void Logger::logMessage(std::string message)
{
    message = "NucleoYokeFF plugin: " + message + "\n";
    XPLMDebugString(message.c_str());
}

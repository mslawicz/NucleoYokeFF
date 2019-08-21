
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include <string.h>
#if IBM
#include <windows.h>
#endif
#if LIN
#include <GL/gl.h>
#elif __GNUC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifndef XPLM300
#error This is made to be compiled against the XPLM300 SDK
#endif


PLUGIN_API int XPluginStart(
    char* outName,
    char* outSig,
    char* outDesc)
{
    const char* pluginName = "HelloWorld3Plugin";
    strcpy_s(outName, sizeof(pluginName), pluginName);
    const char* pluginSig = "xpsdk.examples.helloworld3plugin";
    strcpy_s(outName, sizeof(pluginSig), pluginSig);
    const char* pluginDesc = "A Hello World plug-in for the XPLM300 SDK.";
    strcpy_s(outName, sizeof(pluginDesc), pluginDesc);

    return true;
}

PLUGIN_API void	XPluginStop(void)
{

}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) { }
#include "plugin.h"
#include "utilities.h"
#include "unifly.h"

#include "XPMPMultiplayer.h"

std::unique_ptr<unifly::UniFly> environment;

PLUGIN_API int XPluginStart(char* outName, char* outSignature, char* outDescription)
{
	strcpy(outName, "UniFly");
	strcpy(outSignature, "gg.unifly.xplm");
	strcpy(outDescription, "UniFly cross platform multiplayer XPLM Adapter");

	try
	{
		XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
		XPMPSetPluginName("UniFly");
		Log("UniFly: XPluginStart");
	}
	catch (const std::exception& e)
	{
		Log("UniFly: Exception in XPluginStart: %s", e.what());
		return 0;
	}
	catch (...)
	{
		return 0;
	}

	return 1;
}

PLUGIN_API int XPluginEnable(void)
{
	try
	{
		if (environment)
		{
			environment.reset();
		}
		environment = std::make_unique<unifly::UniFly>();
		Log("UniFly: XPluginEnable");
	}
	catch (std::exception& e)
	{
		Log("UniFly: Exception in XPluginEnable: %s", e.what());
		return 0;
	}
	catch (...)
	{
		return 0;
	}

	return 1;
}

PLUGIN_API void XPluginDisable(void)
{
	try
	{
		environment->DeleteAllAircraft();
		environment->Shutdown();
		environment.reset();
		XPMPMultiplayerDisable();
		XPMPMultiplayerCleanup();
		Log("UniFly: XPluginDisable");
	}
	catch (std::exception& e)
	{
		Log("UniFly: Exception in XPluginDisable: %s", e.what());
	}
	catch (...)
	{
	}
}

PLUGIN_API void XPluginStop(void)
{
	try
	{
		// XPLMDestroyMenu(PluginMenu);
		// XPLMUnregisterCommandHandler(PttCommand, PttCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(SplitAudioChannelCommand, SplitAudioChannelCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(ToggleMessgePreviewPanelCommnd, ToggleMessagePreviewPanelCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(ToggleNearbyATCWindowCommand, ToggleNearbyATCWindowCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(ToggleMessageConsoleCommand, ToggleMessageConsoleCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(ContactAtcCommand, ContactAtcCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(ToggleDefaultAtisCommand, ToggleDefaultAtisCommandHandler, 0, 0);
		// XPLMUnregisterCommandHandler(ToggleTcasCommand, ToggleTcasCommandHandler, 0, 0);
	}
	catch (const std::exception& e)
	{
		Log("UniFly: Exception in XPluginStop: %s", e.what());
	}
	catch (...)
	{
	}
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void* inParam)
{

}

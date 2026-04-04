#include "plugin.h"
#include "XPLMMenus.h"
#include "utilities.h"
#include "unifly.h"
#include "config.h"

#include "XPMPMultiplayer.h"
#include <cstdio>

namespace unifly {

std::unique_ptr<unifly::UniFly> environment;

void CreateMenuItems();
void EnableMenuItems(int enabled);

PLUGIN_API int XPluginStart(char* outName, char* outSignature, char* outDescription)
{
    strcpy(outName, "UniFly");
	strcpy(outSignature, "gg.unifly.xplm");
	strcpy(outDescription, "UniFly cross platform multiplayer XPLM Adapter");
	LOG_MSG("XPluginStarting");

	unifly::global.MarkXPlaneThread();

	try
	{
		XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
		XPMPSetPluginName("UniFly");
		CreateMenuItems();
		LOG_MSG("XPluginStart. Version = %d", UNIFLY_PLUGIN_VERSION);
	}
	catch (const std::exception& e)
	{
		LOG_MSG("Exception in XPluginStart: %s", e.what());
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
		LOG_MSG("XPluginEnable");
	}
	catch (std::exception& e)
	{
		LOG_MSG("Exception in XPluginEnable: %s", e.what());
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
	    LOG_MSG("XPluginDisabling");
		environment->DeleteAllAircraft();
		environment->Shutdown();
		environment.reset();
		XPMPMultiplayerDisable();
		XPMPMultiplayerCleanup();
		LOG_MSG("XPluginDisable");
	}
	catch (std::exception& e)
	{
		LOG_MSG("Exception in XPluginDisable: %s", e.what());
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
		LOG_MSG("XPluginStop");
	}
	catch (const std::exception& e)
	{
		LOG_MSG("UniFly: Exception in XPluginStop: %s", e.what());
	}
	catch (...)
	{
	}
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void* inParam)
{

}

void MenuHandler(void* mRef, void* iRef)
{
    UpdateMenuItems();
}

void CreateMenuItems() {
    // Commands
   	CommandAircraftLabelShow = XPLMCreateCommand("unifly/aircraft_label_show", "UniFly: Toggle Aircraft Labels");
   	CommandAircraftLabelType = XPLMCreateCommand("unifly/aircraft_label_type", "UniFly: Cycle Aircraft Labels type");
   	CommandAircraftLabelColour = XPLMCreateCommand("unifly/aircraft_label_colour", "UniFly: Cycle Aircraft Labels colour");
   	CommandAircraftLabelRange = XPLMCreateCommand("unifly/aircraft_label_range", "UniFly: Cycle Aircraft Labels range");
   	CommandAircraftLabelVisibilityCutoff = XPLMCreateCommand("unifly/aircraft_label_visibility_cutoff", "UniFly: Toggle Aircraft Labels Visibility Cutoff");

    // Menus
    PluginMenuUniFlyIdx = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "UniFly", nullptr, 0);
    PluginMenuUniFly = XPLMCreateMenu("UniFly", XPLMFindPluginsMenu(), PluginMenuUniFlyIdx, MenuHandler, nullptr);

    PluginMenuAircraftLabelIdx = XPLMAppendMenuItem(PluginMenuUniFly, "Aircraft Labels", nullptr, 0);
    PluginMenuAircraftLabel = XPLMCreateMenu("Aircraft Labels", PluginMenuUniFly, PluginMenuAircraftLabelIdx, nullptr, nullptr);

    // Menu Commands
    MenuAircraftLabelShow = XPLMAppendMenuItemWithCommand(PluginMenuAircraftLabel, "Show", CommandAircraftLabelShow);
    MenuAircraftLabelType = XPLMAppendMenuItemWithCommand(PluginMenuAircraftLabel, "Type", CommandAircraftLabelType);
    MenuAircraftLabelColour = XPLMAppendMenuItemWithCommand(PluginMenuAircraftLabel, "Colour", CommandAircraftLabelColour);
    MenuAircraftLabelRange = XPLMAppendMenuItemWithCommand(PluginMenuAircraftLabel, "Range", CommandAircraftLabelRange);
    MenuAircraftLabelVisibilityCutoff = XPLMAppendMenuItemWithCommand(PluginMenuAircraftLabel, "Visibility Cutoff", CommandAircraftLabelVisibilityCutoff);

    EnableMenuItems(false);
}

void EnableMenuItems(int enabled) {
    XPLMEnableMenuItem(PluginMenuAircraftLabel, MenuAircraftLabelShow, enabled);
    XPLMEnableMenuItem(PluginMenuAircraftLabel, MenuAircraftLabelType, false); //TODO: Coming soon
    XPLMEnableMenuItem(PluginMenuAircraftLabel, MenuAircraftLabelColour, false); //TODO: Coming soon
    XPLMEnableMenuItem(PluginMenuAircraftLabel, MenuAircraftLabelRange, enabled);
    XPLMEnableMenuItem(PluginMenuAircraftLabel, MenuAircraftLabelVisibilityCutoff, enabled);
}

void UpdateMenuItems()
{
    static char aircraftLabelType[64];
    static char aircraftLabelColour[64];
    static char aircraftLabelRange[64];

    snprintf(aircraftLabelType, sizeof(aircraftLabelType), "Type: %s", AircraftLabelTypeToString(unifly::Config::GetInstance().GetAircraftLabelType()));
    snprintf(aircraftLabelColour, sizeof(aircraftLabelColour), "Colour: %s", AircraftLabelColourToString(unifly::Config::GetInstance().GetAircraftLabelColour()));
    snprintf(aircraftLabelRange, sizeof(aircraftLabelRange), "Range: %s", AircraftLabelRangeToString(unifly::Config::GetInstance().GetAircraftLabelRange()));

    XPLMSetMenuItemName(PluginMenuAircraftLabel, MenuAircraftLabelShow, unifly::Config::GetInstance().GetAircraftLabelShow() ? "Show: On" : "Show: Off", 0);
    XPLMSetMenuItemName(PluginMenuAircraftLabel, MenuAircraftLabelType, aircraftLabelType, 0);
    XPLMSetMenuItemName(PluginMenuAircraftLabel, MenuAircraftLabelColour, aircraftLabelColour, 0);
    XPLMSetMenuItemName(PluginMenuAircraftLabel, MenuAircraftLabelRange, aircraftLabelRange, 0);
	XPLMSetMenuItemName(PluginMenuAircraftLabel, MenuAircraftLabelVisibilityCutoff, unifly::Config::GetInstance().GetAircraftLabelVisibilityCutoff() ? "Visibility Cutoff: On" : "Visibility Cutoff: Off", 0);
}

void EnactMenuItems() {
    XPMPEnableAircraftLabels(Config::GetInstance().GetAircraftLabelShow());
    XPMPSetAircraftLabelDist(AircraftLabelRangeValue(Config::GetInstance().GetAircraftLabelRange()), Config::GetInstance().GetAircraftLabelVisibilityCutoff());
    // Colour and type are enacted elsewhere TODO
}

int HandleCommandAircraftLabelShow(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref)
{
    if (inPhase == xplm_CommandEnd) {
        auto* instance = static_cast<UniFly*>(ref);
   		if (instance) {
   			auto next = !unifly::Config::GetInstance().GetAircraftLabelShow();
   			unifly::Config::GetInstance().SetAircraftLabelShow(next);
   			unifly::Config::GetInstance().SaveConfig(instance);

            EnactMenuItems();
        }
    }
    return 0;

}

int HandleCommandAircraftLabelType(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref)
{

    if (inPhase == xplm_CommandEnd) {
        auto* instance = static_cast<UniFly*>(ref);
   		if (instance) {
            auto next = AircraftLabelTypeNext(Config::GetInstance().GetAircraftLabelType());
   			unifly::Config::GetInstance().SetAircraftLabelShow(next);
   			unifly::Config::GetInstance().SaveConfig(instance);

            EnactMenuItems();
        }
    }
    return 0;
}

int HandleCommandAircraftLabelColour(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref)
{

    if (inPhase == xplm_CommandEnd) {
        auto* instance = static_cast<UniFly*>(ref);
   		if (instance) {
         auto next = AircraftLabelColourNext(Config::GetInstance().GetAircraftLabelColour());
			unifly::Config::GetInstance().SetAircraftLabelColour(next);
			unifly::Config::GetInstance().SaveConfig(instance);

			EnactMenuItems();
        }
    }
    return 0;
}

int HandleCommandAircraftLabelRange(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref)
{

    if (inPhase == xplm_CommandEnd) {
        auto* instance = static_cast<UniFly*>(ref);
   		if (instance) {
            auto next = AircraftLabelRangeNext(Config::GetInstance().GetAircraftLabelRange());
			unifly::Config::GetInstance().SetAircraftLabelRange(next);
			unifly::Config::GetInstance().SaveConfig(instance);

			EnactMenuItems();
        }
    }
    return 0;
}

int HandleCommandAircraftLabelVisibilityCutoff(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref)
{
    if (inPhase == xplm_CommandEnd) {
        auto* instance = static_cast<UniFly*>(ref);
   		if (instance) {
 			auto next = !unifly::Config::GetInstance().GetAircraftLabelVisibilityCutoff();
 			unifly::Config::GetInstance().SetAircraftLabelVisibilityCutoff(next);
 			unifly::Config::GetInstance().SaveConfig(instance);

            EnactMenuItems();
        }
    }
    return 0;
}


}

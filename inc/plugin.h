#pragma once

#include "XPLMUtilities.h"
namespace unifly
{

static void MenuHandler(void* mRef, void* iRef);

void UpdateMenuItems();
void EnableMenuItems(int enabled);

inline XPLMCommandRef CommandAircraftLabelShow = NULL;
inline XPLMCommandRef CommandAircraftLabelType = NULL;
inline XPLMCommandRef CommandAircraftLabelColour = NULL;
inline XPLMCommandRef CommandAircraftLabelRange = NULL;
inline XPLMCommandRef CommandAircraftLabelVisibilityCutoff = NULL;

int HandleCommandAircraftLabelShow(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref);
int HandleCommandAircraftLabelType(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref);
int HandleCommandAircraftLabelColour(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref);
int HandleCommandAircraftLabelRange(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref);
int HandleCommandAircraftLabelVisibilityCutoff(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref);

inline int PluginMenuUniFlyIdx;
inline XPLMMenuID PluginMenuUniFly;

inline int PluginMenuAircraftLabelIdx;
inline XPLMMenuID PluginMenuAircraftLabel;

static int MenuAircraftLabelShow = 0;
static int MenuAircraftLabelType = 0;
static int MenuAircraftLabelColour = 0;
static int MenuAircraftLabelRange = 0;
static int MenuAircraftLabelVisibilityCutoff = 0;

}

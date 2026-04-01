#pragma once

static void MenuHandler(void* mRef, void* iRef);

inline XPLMCommandRef ToggleAircraftLabelsCommand = NULL;
inline int ToggleAircraftLabelsCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* inRefcon);

inline int PluginMenuIdx;
inline XPLMMenuID PluginMenu;
static int MenuToggleAircraftLabels = 0;

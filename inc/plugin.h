#pragma once

namespace unifly
{

static void MenuHandler(void* mRef, void* iRef);

void UpdateMenuItems();
void EnableMenuItems(int enabled);

inline XPLMCommandRef ToggleAircraftLabelsCommand = NULL;
int HandleToggleAircraftLabelsCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void* ref);

inline int PluginMenuIdx;
inline XPLMMenuID PluginMenu;
static int MenuToggleAircraftLabels = 0;

}

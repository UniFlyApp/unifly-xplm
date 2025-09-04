#include <string>

#include "XPLMUtilities.h"

inline float GetNetworkTime()
{
	static XPLMDataRef drNetworkTime = nullptr;
	if (!drNetworkTime)
	{
		drNetworkTime = XPLMFindDataRef("sim/network/misc/network_time_sec");
	}
	return XPLMGetDataf(drNetworkTime);
}

inline const char* Logger(const char* msg, va_list args)
{
	static char buf[2048];

	float secs = GetNetworkTime();
	const unsigned hours = unsigned(secs / 3600.0f);
	secs -= hours * 3600.0f;
	const unsigned mins = unsigned(secs / 60.0f);
	secs -= mins * 60.0f;

	snprintf(buf, sizeof(buf), "%u:%02u:%06.3f %s:", hours, mins, secs, "UniFly");
	if (args)
	{
		vsnprintf(&buf[strlen(buf)], sizeof(buf) - strlen(buf) - 1, msg, args);
	}
	size_t length = strlen(buf);
	if (buf[length - 1] != '\n')
	{
		buf[length] = '\n';
		buf[length + 1] = 0;
	}
	return buf;
}

/// %d → signed decimal int
/// %u → unsigned int
/// %f → float/double
/// %s → C-string
/// %p → pointer
/// %x / %X → hex
void Log(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	XPLMDebugString(Logger(msg, args));
	va_end(args);
}

std::string GetPluginPath()
{
	XPLMPluginID myId = XPLMGetMyID();
	char buffer[2048];
	XPLMGetPluginInfo(myId, nullptr, buffer, nullptr, nullptr);
	char* path = XPLMExtractFileAndPath(buffer);
	return std::string(buffer, 0, path - buffer) + "/../";
}

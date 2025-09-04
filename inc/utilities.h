#pragma once

void Log (const char* szMsg, ... );

std::string GetPluginPath();

inline char* strScpy(char* dest, const char* src, size_t size)
{
	strncpy(dest, src, size);
	dest[size - 1] = 0;
	return dest;
}

#pragma once

namespace unifly {

std::string GetPluginPath();

inline char* strScpy(char* dest, const char* src, size_t size)
{
	strncpy(dest, src, size);
	dest[size - 1] = 0;
	return dest;
}

//
// MARK: Logging Support
//

/// @brief To apply printf-style warnings to our functions.
/// @see Taken from imgui.h's definition of IM_FMTARGS
#if defined(__clang__) || defined(__GNUC__)
#define XPMP2_FMTARGS(FMT)  __attribute__((format(printf, FMT, FMT+1)))
#else
#define XPMP2_FMTARGS(FMT)
#endif

/// Returns ptr to static buffer filled with formatted log string
const char* LogGetString ( const char* szFile, int ln, const char* szFunc, const char* szMsg, va_list args );

/// Log Text to log file
void LogMsg ( const char* szFile, int ln, const char* szFunc, const char* szMsg, ... ) XPMP2_FMTARGS(4);

void FlushMsgs ();

//
// MARK: Logging macros
//

/// @brief Log a message if lvl is greater or equal currently defined log level
/// @note First parameter after lvl must be the message text,
///       which can be a format string with its parameters following like in sprintf
#define LOG_MSG(...)  {                                         \
    LogMsg(__FILE__, __LINE__, __func__, __VA_ARGS__);       \
}

/// @brief Log a message about matching if logging of model matching is enabled
/// @note First parameter after lvl must be the message text,
///       which can be a format string with its parameters following like in sprintf
#define LOG_MATCHING(lvl,...)  {                                    \
    if (XPMP2::glob.bLogMdlMatch && lvl >= glob.logLvl)             \
    {LogMsg(__FILE__, __LINE__, __func__, lvl, __VA_ARGS__);}       \
}

/// @brief Throws an exception using XPMP2Error
/// @note First parameter after lvl must be the message text,
///       which can be a format string with its parameters following like in sprintf
#define THROW_ERROR(...)                                            \
throw UniFlyError(__FILE__, __LINE__, __func__, __VA_ARGS__);

/// @brief Throw in an assert-style (logging takes place in XPMP2Error constructor)
/// @note This conditional check _always_ takes place, independend of any build or logging settings!
#define LOG_ASSERT(cond)                                            \
    if (!(cond)) {                                                  \
        THROW_ERROR("ASSERT FAILED: %s",#cond);                     \
    }


//
// MARK: Exception class
//

class UniFlyError : public std::logic_error {
protected:
    std::string fileName;
    int ln;
    std::string funcName;
    std::string msg;
public:
    /// Constructor puts together a formatted exception text
    UniFlyError (const char* szFile, int ln, const char* szFunc, const char* szMsg, ...);
public:
    /// returns msg.c_str()
    virtual const char* what() const noexcept;

public:
    // copy/move constructor/assignment as per default
    UniFlyError (const UniFlyError& o) = default;
    UniFlyError (UniFlyError&& o) = default;
    UniFlyError& operator = (const UniFlyError& o) = default;
    UniFlyError& operator = (UniFlyError&& o) = default;
};

} // namespace unifly

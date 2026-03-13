#include "utilities.h"
#include <string>

#include "XPLMUtilities.h"
#include "unifly.h"

namespace unifly {

Global global;

std::string GetPluginPath()
{
	XPLMPluginID myId = XPLMGetMyID();
	char buffer[2048];
	XPLMGetPluginInfo(myId, nullptr, buffer, nullptr, nullptr);
	char* path = XPLMExtractFileAndPath(buffer);
	return std::string(buffer, 0, path - buffer) + "/../";
}


//
//MARK: Log
//


/// The temporary store for messages coming from worker threads
static std::queue<std::string> qMsgs;

/// Lock to control access to qMsgs
static std::mutex mtxMsgs;
/// atomic flag for faster test if there are any msgs to be flushed
static std::atomic_flag fMsgsEmpty;

// returns ptr to static buffer filled with log string
const char* LogGetString (const char* szPath, int ln, const char* szFunc,
                          const char* szMsg, va_list args )
{
     static char aszMsg[2048];

    // prepare timestamp
    snprintf(aszMsg, sizeof(aszMsg), "UniFly: ");

    // append given message
    if (args) {
        vsnprintf(&aszMsg[strlen(aszMsg)],
                  sizeof(aszMsg)-strlen(aszMsg)-1,      // we save one char for the CR
                  szMsg,
                  args);
    }

    // ensure there's a trailing CR
    size_t l = strlen(aszMsg);
    if ( aszMsg[l-1] != '\n' )
    {
        aszMsg[l]   = '\n';
        aszMsg[l+1] = 0;
    }

    // return the static buffer
    return aszMsg;
}



// Flush all pending messages to Log.txt
void FlushMsgs ()
{
    // only if the flag says there's something waiting will we do the more expensive lock operation
    if (!fMsgsEmpty.test_and_set())
    {
        LOG_ASSERT(unifly::global.IsXPThread());

        std::lock_guard<std::mutex> lock(mtxMsgs);  // access to queue guarded by lock
        while (!qMsgs.empty()) {                    // flush all waiting queue entries out to Log.txt
            XPLMDebugString(qMsgs.front().c_str());
            qMsgs.pop();
        }
    }
}


// Store a message for immediate (XP thread) or later output (worker thread)
void PushMsg (const std::string& sMsg)
{
    // If we are in XP's main thread we can just write...but only after waiting msgs

    if (unifly::global.IsXPThread()) {
        FlushMsgs();
        XPLMDebugString(sMsg.c_str());
    }
    // In a worker thread we have to add the message to the queue
    else {
        std::lock_guard<std::mutex> lock(mtxMsgs);  // access to queue guarded by lock
        qMsgs.push(sMsg);                           // add msg to queue
        fMsgsEmpty.clear();                         // clear the atomic flag...the queue is no longer empty
    }
}

// Log Text to log file
void LogMsg ( const char* szPath, int ln, const char* szFunc, const char* szMsg, ... )
{
    va_list args;

    va_start (args, szMsg);
    PushMsg ( LogGetString(szPath, ln, szFunc, szMsg, args) );
    va_end (args);
}


//
// MARK: Exception
//

// standard constructor
UniFlyError::UniFlyError (const char* _szFile, int _ln, const char* _szFunc,
                        const char* _szMsg, ...) :
std::logic_error(LogGetString(_szFile, _ln, _szFunc, _szMsg, NULL)),
fileName(_szFile), ln(_ln), funcName(_szFunc)
{
    va_list args;
    va_start (args, _szMsg);
    msg = LogGetString(_szFile, _ln, _szFunc, _szMsg, args);
    va_end (args);

    // write to log
    PushMsg ( msg.c_str() );
}

const char* UniFlyError::what() const noexcept
{
    return msg.c_str();
}


}

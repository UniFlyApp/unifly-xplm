void Log (const char* szMsg, ... )
{
    char buf[512];
    va_list args;
    // Write all the variable parameters
    va_start (args, szMsg);
    std::vsnprintf(buf, sizeof(buf)-2, szMsg, args);
    va_end (args);
    std::strcat(buf, "\n");
    // write to log (flushed immediately -> expensive!)
    XPLMDebugString(buf);
}

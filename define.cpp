#include "define.h"

const char* EventTypeString(EEventType event)
{
    switch (event)
    {
        case ECONNECT: return "ECONNECT";
        case EDISCONNECT: return "EDISCONNECT";
        case ERECVMSG: return "ERECVMSG";
        case ELAGNOTIFY: return "ELAGNOTIFY";
        default: return "UNKNOWN";
    }
}

unsigned long long GetCurrentTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long milli = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return milli;
}

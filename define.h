#ifndef _DEFINE_
#define _DEFINE_

#include <sys/time.h>
#include <stdint.h>
#include <memory>

#define KCP_CONNECTION_TIMEOUT_TIME (10*1000)   // default is 10 seconds

struct IKCPCB;
typedef struct IKCPCB ikcpcb;
typedef uint32_t kcp_conv_t;

enum EEventType
{
    ECONNECT,
    EDISCONNECT,
    ERECVMSG,
    ELAGNOTIFY,
    
    EEVENTCOUNT
};

enum ESockState
{
    ESOCKINIT,
    ESOCKCONNECTING,
    ESOCKCONNECTED,
    ESOCKDISCONNECT
};

const char* EventTypeString(EEventType event);
typedef void (event_callback_t)(kcp_conv_t covn, EEventType event_type, std::string& string);

unsigned long long GetCurrentTime();

#endif

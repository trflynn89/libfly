#pragma once

#include "fly/fly.h"

#if defined(FLY_WINDOWS)
    #include <WinSock.h>
#endif

namespace fly {

#if defined(FLY_WINDOWS)
    typedef SOCKET socket_type;
#elif defined(FLY_LINUX)
    typedef int socket_type;
#else
    #error Unknown socket type
#endif

}

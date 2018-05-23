#pragma once

#include <cstdint>

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

typedef uint32_t address_type;
typedef uint16_t port_type;

}

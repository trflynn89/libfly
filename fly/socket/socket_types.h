#pragma once

#include "fly/fly.h"

#include <cstdint>

#if defined(FLY_WINDOWS)
#    include <WinSock.h>
#endif

namespace fly {

#if defined(FLY_WINDOWS)
typedef SOCKET socket_type;
#elif defined(FLY_LINUX)
typedef int socket_type;
#else
#    error Unknown socket type
#endif

typedef uint32_t address_type;
typedef uint16_t port_type;

/**
 * Types of supported sockets.
 */
enum class Protocol : uint8_t
{
    TCP,
    UDP
};

/**
 * Supported options for binding sockets.
 */
enum class BindOption : uint8_t
{
    SingleUse,
    AllowReuse
};

/**
 * TCP socket connection states.
 */
enum class ConnectedState : uint8_t
{
    Disconnected,
    Connecting,
    Connected
};

} // namespace fly

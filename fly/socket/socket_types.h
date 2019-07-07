#pragma once

#include "fly/fly.h"
#include "fly/logger/logger.h"

#include <cstdint>

#if defined(FLY_WINDOWS)
#    include <WinSock.h>
#endif

#define SLOGD(handle, format, ...) LOGD("[%d] " format, handle, ##__VA_ARGS__)
#define SLOGI(handle, format, ...) LOGI("[%d] " format, handle, ##__VA_ARGS__)
#define SLOGW(handle, format, ...) LOGW("[%d] " format, handle, ##__VA_ARGS__)
#define SLOGS(handle, format, ...) LOGS("[%d] " format, handle, ##__VA_ARGS__)
#define SLOGE(handle, format, ...) LOGE("[%d] " format, handle, ##__VA_ARGS__)

namespace fly {

#if defined(FLY_WINDOWS)
typedef SOCKET socket_type;
#elif defined(FLY_LINUX)
typedef int socket_type;
#else
#    error Unknown socket type.
#endif

typedef std::uint32_t address_type;
typedef std::uint16_t port_type;

/**
 * Types of supported sockets.
 */
enum class Protocol : std::uint8_t
{
    TCP,
    UDP
};

/**
 * Supported options for binding sockets.
 */
enum class BindOption : std::uint8_t
{
    SingleUse,
    AllowReuse
};

/**
 * TCP socket connection states.
 */
enum class ConnectedState : std::uint8_t
{
    Disconnected,
    Connecting,
    Connected
};

} // namespace fly

#pragma once

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"

#include <cstdint>

#if defined(FLY_WINDOWS)
#    include <WinSock2.h>
#endif

/**
 * Wrapper logging macros to format logs with socket handles in a consistent manner.
 */
#define SLOGD(handle, ...)                                                                         \
    LOGD("[{}] " FLY_FORMAT_STRING(__VA_ARGS__), handle FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGI(handle, ...)                                                                         \
    LOGI("[{}] " FLY_FORMAT_STRING(__VA_ARGS__), handle FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGW(handle, ...)                                                                         \
    LOGW("[{}] " FLY_FORMAT_STRING(__VA_ARGS__), handle FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGS(handle, ...)                                                                         \
    LOGS("[{}] " FLY_FORMAT_STRING(__VA_ARGS__), handle FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGE(handle, ...)                                                                         \
    LOGE("[{}] " FLY_FORMAT_STRING(__VA_ARGS__), handle FLY_FORMAT_ARGS(__VA_ARGS__))

namespace fly {

#if defined(FLY_LINUX) || defined(FLY_MACOS)
using socket_type = int;
#elif defined(FLY_WINDOWS)
using socket_type = SOCKET;
#else
#    error Unknown socket type.
#endif

using address_type = std::uint32_t;
using port_type = std::uint16_t;

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

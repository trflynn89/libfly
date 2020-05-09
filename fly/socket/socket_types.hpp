#pragma once

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"

#include <cstdint>

#if defined(FLY_WINDOWS)
#    include <WinSock.h>
#endif

/**
 * Wrapper logging macros to format logs with socket handles in a consistent
 * manner.
 *
 * The private macros used internally insert commas only if one is needed, which
 * the formatter doesn't handle.
 */
// clang-format off
#define SLOGD(handle, ...)                                                     \
    LOGD(                                                                      \
        "[%d] " _FLY_FORMAT_STRING(__VA_ARGS__),                               \
        handle                                                                 \
        _FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGI(handle, ...)                                                     \
    LOGI(                                                                      \
        "[%d] " _FLY_FORMAT_STRING(__VA_ARGS__),                               \
        handle                                                                 \
        _FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGW(handle, ...)                                                     \
    LOGW(                                                                      \
        "[%d] " _FLY_FORMAT_STRING(__VA_ARGS__),                               \
        handle                                                                 \
        _FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGS(handle, ...)                                                     \
    LOGS(                                                                      \
        "[%d] " _FLY_FORMAT_STRING(__VA_ARGS__),                               \
        handle                                                                 \
        _FLY_FORMAT_ARGS(__VA_ARGS__))
#define SLOGE(handle, ...)                                                     \
    LOGE(                                                                      \
        "[%d] " _FLY_FORMAT_STRING(__VA_ARGS__),                               \
        handle                                                                 \
        _FLY_FORMAT_ARGS(__VA_ARGS__))
// clang-format on

namespace fly {

#if defined(FLY_WINDOWS)
using socket_type = SOCKET;
#elif defined(FLY_LINUX)
using socket_type = int;
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

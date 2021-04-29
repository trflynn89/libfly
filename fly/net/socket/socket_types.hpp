#pragma once

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"
#include "fly/types/string/formatters.hpp"

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

namespace fly::net {

#if defined(FLY_LINUX) || defined(FLY_MACOS)
using socket_type = int;
#elif defined(FLY_WINDOWS)
using socket_type = SOCKET;
#else
#    error Unknown socket type.
#endif

using port_type = std::uint16_t;

/**
 * Supported modes for IO processing.
 */
enum class IOMode : std::uint8_t
{
    Synchronous,
    Asynchronous,
};

/**
 * Supported modes for binding sockets.
 */
enum class BindMode : std::uint8_t
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

} // namespace fly::net

//==================================================================================================
template <>
struct fly::Formatter<fly::net::IOMode> : public fly::Formatter<std::uint8_t>
{
    /**
     * Format an IO processing mode.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param mode The IO processing mode to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(fly::net::IOMode mode, FormatContext &context)
    {
        fly::Formatter<std::uint8_t>::format(static_cast<std::uint8_t>(mode), context);
    }
};

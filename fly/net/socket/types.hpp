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
#define SLOGD(handle, format, ...) LOGD("[{}] " format, handle __VA_OPT__(, ) __VA_ARGS__)
#define SLOGI(handle, format, ...) LOGI("[{}] " format, handle __VA_OPT__(, ) __VA_ARGS__)
#define SLOGW(handle, format, ...) LOGW("[{}] " format, handle __VA_OPT__(, ) __VA_ARGS__)
#define SLOGS(handle, format, ...) LOGS("[{}] " format, handle __VA_OPT__(, ) __VA_ARGS__)
#define SLOGE(handle, format, ...) LOGE("[{}] " format, handle __VA_OPT__(, ) __VA_ARGS__)

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
struct fly::string::Formatter<fly::net::IOMode> : public fly::string::Formatter<std::uint8_t>
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
        fly::string::Formatter<std::uint8_t>::format(static_cast<std::uint8_t>(mode), context);
    }
};

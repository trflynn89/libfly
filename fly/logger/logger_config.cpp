#include "fly/logger/logger_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
LoggerConfig::LoggerConfig() noexcept :
    m_defaultMaxLogFileSize(U64(20 << 20)),
    m_defaultMaxMessageSize(U64(256)),
    m_defaultQueueWaitTime(I64(100))
{
}

//==============================================================================
std::uintmax_t LoggerConfig::MaxLogFileSize() const noexcept
{
    return GetValue<std::uintmax_t>(
        "max_log_file_size", m_defaultMaxLogFileSize);
}

//==============================================================================
std::uint32_t LoggerConfig::MaxMessageSize() const noexcept
{
    return GetValue<std::uint32_t>(
        "max_message_size", m_defaultMaxMessageSize);
}

//==============================================================================
std::chrono::milliseconds LoggerConfig::QueueWaitTime() const noexcept
{
    return std::chrono::milliseconds(GetValue<std::chrono::milliseconds::rep>(
        "queue_wait_time", m_defaultQueueWaitTime));
}

} // namespace fly

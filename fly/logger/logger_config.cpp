#include "fly/logger/logger_config.h"

#include "fly/types/literals.h"

namespace fly {

//==============================================================================
LoggerConfig::LoggerConfig() noexcept :
    m_defaultMaxLogFileSize(20_u64 << 20),
    m_defaultMaxMessageSize(256_u32),
    m_defaultQueueWaitTime(100_i64)
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
    return GetValue<std::uint32_t>("max_message_size", m_defaultMaxMessageSize);
}

//==============================================================================
std::chrono::milliseconds LoggerConfig::QueueWaitTime() const noexcept
{
    return std::chrono::milliseconds(GetValue<std::chrono::milliseconds::rep>(
        "queue_wait_time", m_defaultQueueWaitTime));
}

} // namespace fly

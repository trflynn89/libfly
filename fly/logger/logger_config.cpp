#include "fly/logger/logger_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
LoggerConfig::LoggerConfig() :
    m_defaultMaxLogFileSize(U64(20 << 20)),
    m_defaultMaxMessageSize(U64(256)),
    m_defaultQueueWaitTime(I64(100))
{
}

//==============================================================================
std::string LoggerConfig::GetName()
{
    return "logger";
}

//==============================================================================
size_t LoggerConfig::MaxLogFileSize() const
{
    return GetValue<size_t>("max_log_file_size", m_defaultMaxLogFileSize);
}

//==============================================================================
size_t LoggerConfig::MaxMessageSize() const
{
    return GetValue<size_t>("max_message_size", m_defaultMaxMessageSize);
}

//==============================================================================
std::chrono::milliseconds LoggerConfig::QueueWaitTime() const
{
    return std::chrono::milliseconds(GetValue<std::chrono::milliseconds::rep>(
        "queue_wait_time", m_defaultQueueWaitTime));
}

} // namespace fly

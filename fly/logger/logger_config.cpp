#include "fly/logger/logger_config.h"

namespace fly {

//==============================================================================
std::string LoggerConfig::GetName()
{
    return "logger";
}

//==============================================================================
size_t LoggerConfig::MaxLogFileSize() const
{
    return GetValue<size_t>("max_log_file_size", U64(20 << 20));
}

//==============================================================================
size_t LoggerConfig::MaxMessageSize() const
{
    return GetValue<size_t>("max_message_size", U64(256));
}

//==============================================================================
std::chrono::seconds LoggerConfig::QueueWaitTime() const
{
    return std::chrono::seconds(
        GetValue<std::chrono::seconds::rep>("queue_wait_time", I64(1))
    );
}

}

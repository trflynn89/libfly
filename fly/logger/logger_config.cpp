#include "fly/logger/logger_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==================================================================================================
LoggerConfig::LoggerConfig() noexcept :
    m_default_max_log_file_size(20_u64 << 20),
    m_default_max_message_size(256_u32),
    m_default_queue_wait_time(100_i64)
{
}

//==================================================================================================
std::uintmax_t LoggerConfig::max_log_file_size() const noexcept
{
    return get_value<std::uintmax_t>("max_log_file_size", m_default_max_log_file_size);
}

//==================================================================================================
std::uint32_t LoggerConfig::max_message_size() const noexcept
{
    return get_value<std::uint32_t>("max_message_size", m_default_max_message_size);
}

//==================================================================================================
std::chrono::milliseconds LoggerConfig::queue_wait_time() const noexcept
{
    return std::chrono::milliseconds(
        get_value<std::chrono::milliseconds::rep>("queue_wait_time", m_default_queue_wait_time));
}

} // namespace fly

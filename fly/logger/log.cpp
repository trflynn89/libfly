#include "fly/logger/log.hpp"

#include "fly/logger/logger_config.hpp"

namespace fly {

namespace {

    constexpr const char s_record_separator = '\x1e';
    constexpr const char s_unit_separator = '\x1f';

} // namespace

//==================================================================================================
Log::Log(Log &&log) noexcept :
    m_index(std::move(log.m_index)),
    m_level(std::move(log.m_level)),
    m_time(std::move(log.m_time)),
    m_file(std::move(log.m_file)),
    m_function(std::move(log.m_function)),
    m_line(std::move(log.m_line)),
    m_message(std::move(log.m_message))
{
}

//==================================================================================================
Log::Log(const std::shared_ptr<LoggerConfig> &config, std::string &&message) noexcept :
    m_message(std::move(message), 0, config->max_message_size())
{
}

//==================================================================================================
Log &Log::operator=(Log &&log) noexcept
{
    m_index = std::move(log.m_index);
    m_level = std::move(log.m_level);
    m_time = std::move(log.m_time);
    m_file = std::move(log.m_file);
    m_function = std::move(log.m_function);
    m_line = std::move(log.m_line);
    m_message = std::move(log.m_message);

    return *this;
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, const Log &log)
{
    stream << log.m_index << s_unit_separator;
    stream << log.m_level << s_unit_separator;
    stream << log.m_time << s_unit_separator;
    stream << log.m_file << s_unit_separator;
    stream << log.m_function << s_unit_separator;
    stream << log.m_line << s_unit_separator;
    stream << log.m_message << s_record_separator;

    return stream;
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, const Log::Level &level)
{
    stream << static_cast<int>(level);
    return stream;
}

} // namespace fly

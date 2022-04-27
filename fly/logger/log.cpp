#include "fly/logger/log.hpp"

namespace fly::logger {

namespace {

    constexpr char const s_record_separator = '\x1e';
    constexpr char const s_unit_separator = '\x1f';

} // namespace

//==================================================================================================
Log::Log(Trace &&trace, std::string &&message, std::uint32_t max_message_size) noexcept :
    m_trace(std::move(trace)),
    m_message(std::move(message), 0, max_message_size)
{
}

//==================================================================================================
Log::Log(Log &&log) noexcept :
    m_index(std::move(log.m_index)),
    m_level(std::move(log.m_level)),
    m_trace(std::move(log.m_trace)),
    m_time(std::move(log.m_time)),
    m_message(std::move(log.m_message))
{
}

//==================================================================================================
Log &Log::operator=(Log &&log) noexcept
{
    m_index = std::move(log.m_index);
    m_level = std::move(log.m_level);
    m_trace = std::move(log.m_trace);
    m_time = std::move(log.m_time);
    m_message = std::move(log.m_message);

    return *this;
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, Log const &log)
{
    stream << log.m_index << s_unit_separator;
    stream << static_cast<std::uint8_t>(log.m_level) << s_unit_separator;
    stream << log.m_time << s_unit_separator;
    stream << fly::string::format("{}", log.m_trace) << s_unit_separator;
    stream << log.m_message << s_record_separator;

    return stream;
}

} // namespace fly::logger

#include "fly/logger/log.hpp"

#include "fly/types/string/string.hpp"

namespace fly::logger {

namespace {

    constexpr const char s_record_separator = '\x1e';
    constexpr const char s_unit_separator = '\x1f';

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
std::ostream &operator<<(std::ostream &stream, const Log &log)
{
    stream << log.m_index << s_unit_separator;
    stream << log.m_level << s_unit_separator;
    stream << log.m_time << s_unit_separator;
    stream << log.m_trace << s_unit_separator;
    stream << log.m_message << s_record_separator;

    return stream;
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, const Level &level)
{
    stream << static_cast<int>(level);
    return stream;
}

//==================================================================================================
std::ostream &operator<<(std::ostream &stream, const Trace &trace)
{
    if (!trace.m_file.empty() && !trace.m_function.empty())
    {
        stream << fly::String::format("{}:{}:{}", trace.m_file, trace.m_function, trace.m_line);
    }

    return stream;
}

} // namespace fly::logger

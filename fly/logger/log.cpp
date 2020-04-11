#include "fly/logger/log.hpp"

#include "fly/logger/logger_config.hpp"

#include <cstring>

namespace fly {

//==============================================================================
Log::Log() noexcept :
    m_level(Level::NumLevels),
    m_time(-1.0),
    m_line(0),
    m_message()
{
    std::memset(m_file, 0, sizeof(m_file));
    std::memset(m_function, 0, sizeof(m_file));
}

//==============================================================================
Log::Log(Log &&log) noexcept :
    m_level(std::move(log.m_level)),
    m_time(std::move(log.m_time)),
    m_line(std::move(log.m_line)),
    m_message(std::move(log.m_message))
{
    std::memmove(m_file, log.m_file, sizeof(m_file));
    std::memmove(m_function, log.m_function, sizeof(m_file));
}

//==============================================================================
Log::Log(
    const std::shared_ptr<LoggerConfig> &spConfig,
    const std::string &message) noexcept :
    m_level(Level::NumLevels),
    m_time(-1.0),
    m_line(0),
    m_message(message, 0, spConfig->MaxMessageSize())
{
    std::memset(m_file, 0, sizeof(m_file));
    std::memset(m_function, 0, sizeof(m_file));
}

//==============================================================================
Log &Log::operator=(Log &&log) noexcept
{
    m_level = std::move(log.m_level);
    m_time = std::move(log.m_time);
    std::memmove(m_file, log.m_file, sizeof(m_file));
    std::memmove(m_function, log.m_function, sizeof(m_file));
    m_line = std::move(log.m_line);
    m_message = std::move(log.m_message);

    return *this;
}

//==============================================================================
std::ostream &operator<<(std::ostream &stream, const Log &log) noexcept
{
    stream << log.m_level << '\t';
    stream << log.m_time << '\t';
    stream << log.m_file << '\t';
    stream << log.m_function << '\t';
    stream << log.m_line << '\t';
    stream << log.m_message << '\n';

    return stream;
}

//==============================================================================
std::ostream &operator<<(std::ostream &stream, const Log::Level &level) noexcept
{
    stream << static_cast<int>(level);
    return stream;
}

} // namespace fly

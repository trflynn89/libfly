#include "fly/logger/log.h"

#include <cstring>

#include "fly/logger/logger_config.h"

namespace fly {

//==============================================================================
Log::Log() :
    m_level(Level::NumLevels),
    m_time(-1.0),
    m_gameId(-1),
    m_line(0),
    m_message()
{
    ::memset(m_file, 0, sizeof(m_file));
    ::memset(m_function, 0, sizeof(m_file));
}

//==============================================================================
Log::Log(const LoggerConfigPtr &spConfig, const std::string &message) :
    m_level(Level::NumLevels),
    m_time(-1.0),
    m_gameId(-1),
    m_line(0),
    m_message(message, 0, spConfig->MaxMessageSize())
{
    ::memset(m_file, 0, sizeof(m_file));
    ::memset(m_function, 0, sizeof(m_file));
}

//==============================================================================
std::ostream &operator << (std::ostream &stream, const Log &log)
{
    stream << log.m_level << "\t";
    stream << log.m_time << "\t";
    stream << log.m_gameId << "\t";
    stream << log.m_file << "\t";
    stream << log.m_function << "\t";
    stream << log.m_line << "\t";
    stream << log.m_message << "\n";

    return stream;
}

//==============================================================================
std::ostream &operator << (std::ostream &stream, const Log::Level &level)
{
    stream << static_cast<int>(level);
    return stream;
}

}

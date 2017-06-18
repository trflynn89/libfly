#include "fly/logger/log.h"

#include "fly/logger/logger_config.h"

namespace fly {

//==============================================================================
Log::Log(const LoggerConfigPtr &spConfig, const std::string &message) :
    m_message(message, 0, spConfig->MaxMessageSize())
{
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

}

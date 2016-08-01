#include "log.h"

namespace fly {

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

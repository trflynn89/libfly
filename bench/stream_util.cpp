#include "bench/stream_util.hpp"

namespace fly::benchmark {

//==================================================================================================
ScopedStreamModifiers::ScopedStreamModifiers(std::ostream &stream) :
    m_stream(stream),
    m_locale(stream.getloc()),
    m_flags(stream.flags()),
    m_precision(stream.precision())
{
}

//==================================================================================================
ScopedStreamModifiers::~ScopedStreamModifiers()
{
    m_stream.imbue(m_locale);
    m_stream.flags(m_flags);
    m_stream.precision(m_precision);
}

//==================================================================================================
void ScopedStreamModifiers::precision(std::streamsize precision)
{
    m_stream.precision(precision);
}

//==================================================================================================
std::ios::char_type CommaPunctuation::do_thousands_sep() const
{
    return ',';
}

//==================================================================================================
std::string CommaPunctuation::do_grouping() const
{
    return "\3";
}

} // namespace fly::benchmark

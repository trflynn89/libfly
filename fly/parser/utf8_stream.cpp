#include "fly/parser/utf8_stream.hpp"

namespace fly {

//==================================================================================================
UTF8CharStream::UTF8CharStream(std::basic_istream<char> &stream) : UTF8Stream(), m_stream(stream)
{
}

//==================================================================================================
bool UTF8CharStream::good()
{
    return m_stream.good();
}

//==================================================================================================
bool UTF8CharStream::eof()
{
    return m_stream.eof();
}

//==================================================================================================
int UTF8CharStream::peek_internal()
{
    return static_cast<int>(m_stream.peek());
}

//==================================================================================================
int UTF8CharStream::get_internal()
{
    return static_cast<int>(m_stream.get());
}

//==================================================================================================
bool UTF8CharStream::is_eof(int ch)
{
    return static_cast<std::char_traits<char>::int_type>(ch) == std::char_traits<char>::eof();
}

#if !defined(FLY_MACOS)

//==================================================================================================
UTF8Char8Stream::UTF8Char8Stream(std::basic_istream<char8_t> &stream) :
    UTF8Stream(),
    m_stream(stream)
{
}

//==================================================================================================
bool UTF8Char8Stream::good()
{
    return m_stream.good();
}

//==================================================================================================
bool UTF8Char8Stream::eof()
{
    return m_stream.eof();
}

//==================================================================================================
int UTF8Char8Stream::peek_internal()
{
    return static_cast<int>(m_stream.peek());
}

//==================================================================================================
int UTF8Char8Stream::get_internal()
{
    return static_cast<int>(m_stream.get());
}

//==================================================================================================
bool UTF8Char8Stream::is_eof(int ch)
{
    return static_cast<std::char_traits<char8_t>::int_type>(ch) == std::char_traits<char8_t>::eof();
}

#endif // FLY_MACOS

} // namespace fly

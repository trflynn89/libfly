#include "fly/parser/utf8_stream.hpp"

namespace fly {

//==================================================================================================
bool UTF8Stream::eof()
{
    return is_eof(peek_internal());
}

//==================================================================================================
UTF8CharStream::UTF8CharStream(std::basic_istream<char> &stream) :
    UTF8Stream(),
    m_stream_buffer(stream.rdbuf())
{
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
    m_stream_buffer(stream.rdbuf())
{
}

//==================================================================================================
bool UTF8Char8Stream::is_eof(int ch)
{
    return static_cast<std::char_traits<char8_t>::int_type>(ch) == std::char_traits<char8_t>::eof();
}

#endif // FLY_MACOS

} // namespace fly

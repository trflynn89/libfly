#include "fly/types/string/string_exception.hpp"

#include "fly/types/string/string.hpp"

namespace fly {

//==============================================================================
StringException::StringException(
    const char *class_name,
    std::string &&message) noexcept :
    m_message(String::format("%s: %s", class_name, message))
{
}

//==============================================================================
const char *StringException::what() const noexcept
{
    return m_message.c_str();
}

//==============================================================================
UnicodeException::UnicodeException(const char *message) noexcept :
    StringException("UnicodeException", std::string(message))
{
}

//==============================================================================
UnicodeException::UnicodeException(
    const char *message,
    std::uint32_t arg1) noexcept :
    StringException("UnicodeException", String::format(message, arg1))
{
}

//==============================================================================
UnicodeException::UnicodeException(
    const char *message,
    std::uint32_t arg1,
    std::uint32_t arg2) noexcept :
    StringException("UnicodeException", String::format(message, arg1, arg2))
{
}

} // namespace fly

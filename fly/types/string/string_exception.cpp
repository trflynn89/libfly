#include "fly/types/string/string_exception.hpp"

#include "fly/types/string/string.hpp"

namespace fly {

//==================================================================================================
StringException::StringException(const char *class_name, std::string &&message) noexcept :
    m_message(String::format("%s: %s", class_name, message))
{
}

//==================================================================================================
const char *StringException::what() const noexcept
{
    return m_message.c_str();
}

//==================================================================================================
UnicodeException::UnicodeException(std::string &&message) noexcept :
    StringException("UnicodeException", std::move(message))
{
}

} // namespace fly
